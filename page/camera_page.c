#include <page_manager.h>
#include <stdio.h>
#include <ui.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>     // 目录操作相关的头文件

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <jpeglib.h>
#include <linux/fb.h>
#include <pthread.h>

//========================摄像头驱动部分代码========================================
static int fd;							//摄像头文件句柄

static int fd_fb;						//LCD显存文件句柄
static int screen_size;				    //屏幕像素大小
static int LCD_width;					//LCD宽度
static int LCD_height;					//LCD高度
static unsigned char *fbbase = NULL;	//LCD显存地址
static unsigned long line_length;       //LCD一行的长度（字节为单位）
static unsigned int bpp;    			//像素深度bpp

static unsigned char *mmpaddr[4];       //用于存储映射后的首地址
static unsigned int addr_length[4];     //存储映射后空间的大小

pthread_t tid;					        //摄像头画面捕捉线程的tid
static int capture_signal;		        //摄像头拍照信号

#define CAPTURE_STA_RUN  	1
#define CAPTURE_STA_READY  	0

/*******************************************初始化LCD***************************************/
static int LCD_Init(void)
{
	struct fb_var_screeninfo var;   /* Current var */
	struct fb_fix_screeninfo fix;   /* Current fix */
	fd_fb = open("/dev/fb0", O_RDWR);
	if(fd_fb < 0)
	{
		perror("打开LCD失败");
		return -1;
	}
	//获取LCD信息
	ioctl(fd_fb, FBIOGET_VSCREENINFO, &var);//获取屏幕可变信息
	ioctl(fd_fb, FBIOGET_FSCREENINFO, &fix);//获取屏幕固定信息
	//LCD_width  = var.xres * var.bits_per_pixel / 8;
    //pixel_width = var.bits_per_pixel / 8;
    screen_size = var.xres * var.yres * var.bits_per_pixel / 8;
	LCD_width = var.xres;
	LCD_height = var.yres;
	bpp = var.bits_per_pixel;
	line_length = fix.line_length;
	printf("LCD分辨率:%d %d\n",LCD_width, LCD_height);
	printf("bpp: %d\n", bpp);
	fbbase = mmap(NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);//映射
	if (fbbase == (unsigned char *)-1)
    {
        printf("can't mmap\n");
        return -1;
    }
    // memset(fbbase, 0xFF, screen_size);//LCD设置为白色背景
    return 0;
}

static lv_obj_t * Camera_page = NULL;	//屏幕对象
#define FRAME_OBJ_CHILD_ID		1		//代表屏幕父对象的第2个子对象
#define FRAME_SW_CHILD_ID		2		//代表屏幕父对象的第3个子对象
#define LABEL_STATE_CHILD_ID	3		//代表屏幕父对象的第4个子对象

static void my_Btn_return_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_CLICKED)
	{
        // 防止在没有关闭摄像头的情况下，直接返回主页面
		lv_obj_t *Frame_sw = lv_obj_get_child(Camera_page, FRAME_SW_CHILD_ID);	//获取到开关对象的地址
		if(lv_obj_has_state(Frame_sw, LV_STATE_CHECKED))	//如果用户按下返回时仍然是打开摄像头的状态则清除摄像头数据
		{
			int ret;
			ret = pthread_cancel(tid);
			if (ret != 0) {
				// 处理错误
				perror("线程销毁失败！");
				return ;
			}

			int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			//关闭设备
			ret = ioctl(fd, VIDIOC_STREAMOFF, &type);
			if(ret < 0)
				perror("关闭设备失败");

			for(int i = 0; i < 4; i++){
				munmap(mmpaddr[i], addr_length[i]);
			}
				
			close(fd);
		}

		Page("main")->Run(NULL);
	}
}

static int LCD_JPEG_Show(const char *JpegData, int size)
{
	int min_hight = LCD_height, min_width = LCD_width, valid_bytes;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	//错误处理对象与解压对象绑定
	cinfo.err = jpeg_std_error(&jerr);

	//创建解码对象
	jpeg_create_decompress(&cinfo);

	//指定解码数据源
	jpeg_mem_src(&cinfo, (const unsigned char*)JpegData, size);

	//读取图像信息
	jpeg_read_header(&cinfo, TRUE);

	//printf("jpeg图像的大小为：%d*%d\n", cinfo.image_width, cinfo.image_height);

	//设置解码参数
	cinfo.out_color_space = JCS_RGB;//可以不设置默认为RGB

	//cinfo.scale_num = 1;
	//cinfo.scale_denom = 1;设置图像缩放，scale_num/scale_denom缩放比例，默认为1

	//开始解码
	jpeg_start_decompress(&cinfo);
	
	//为缓冲区分配空间
	unsigned char*jpeg_line_buf = malloc(cinfo.output_components * cinfo.output_width);
	unsigned int*fb_line_buf = malloc(line_length);//每个成员4个字节和RGB888对应

	// 判断图像和LCD屏幕分辨率哪个更低，作为图像显示的限制尺寸
	if(cinfo.output_width < min_width)
		min_width = cinfo.output_width;
	if(cinfo.output_height < min_hight)
		min_hight = cinfo.output_height;
	
	// 一行有效字节数，实际写入LCD显存的一行数据大小
	valid_bytes = min_width * bpp / 8;

	// 获取LCD显示的起始地址，计算显示区域的偏移量
	unsigned char *ptr = fbbase + (LCD_width*bpp/8*((600-min_hight)/2));

	// 按行读取图像数据并写入LCD
	while(cinfo.output_scanline < min_hight)
	{
		// 每次读取一行JPEG图像数据
		jpeg_read_scanlines(&cinfo, &jpeg_line_buf, 1);

		//将读取到的BGR888数据转化为RGB888
		unsigned int red, green, blue;
		unsigned int color;  

		for(int i = 0; i < min_width; i++)
		{
			red = jpeg_line_buf[i*3]; // 获取红色通道
			green = jpeg_line_buf[i*3+1]; // 获取绿色通道
			blue = jpeg_line_buf[i*3+2]; // 获取蓝色通道

			// RGB888颜色转换为LCD显示格式（假设为RGB565格式）
			color = red<<16 | green << 8 | blue;

			// 将转换后的颜色值写入LCD缓冲区
			fb_line_buf[i] = color;
		}

		// 将转换后的颜色数据写入LCD显存
		memcpy(ptr+(((1024-min_width)/2)*4), fb_line_buf, valid_bytes);
		ptr += LCD_width*bpp/8; // 更新显存指针到下一行

	}

	//完成解码
	jpeg_finish_decompress(&cinfo);

	//销毁解码对象
	jpeg_destroy_decompress(&cinfo);

	//释放内存
	free(jpeg_line_buf);
	free(fb_line_buf);

	return 0;
}

// 函数：列出指定目录下的所有文件和目录
uint16_t list_files(const char *path) {
    DIR *dir;           // 指向DIR结构的指针，用于表示目录流
    struct dirent *entry; // 指向dirent结构的指针，用于读取目录项
	uint16_t cnt = 0;

    // 打开目录流，如果失败则打印错误信息
    if ((dir = opendir(path)) == NULL) {
        perror("opendir"); // perror函数输出错误信息
        return -1;
    }

    // 循环读取目录中的所有项
    while ((entry = readdir(dir)) != NULL) {
		cnt++;
        // printf("%s:id:%d\n", entry->d_name, cnt); // 打印目录项的名称
    }

    // 关闭目录流
    closedir(dir);

	return cnt;
}

static void *Camera_Read_thread_func(void *data)
{
	int ret;
	struct v4l2_buffer readbuffer;

	while (1)
	{
		//从队列中提取一帧数据
		readbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		ret = ioctl(fd, VIDIOC_DQBUF, &readbuffer);//从缓冲队列获取一帧数据（出队列）

		if(ret < 0)
			perror("获取数据失败");

		// 接收到拍照信号，准备存储照片
		if(capture_signal == CAPTURE_STA_RUN)
		{
			uint16_t file_name_num_id = 0;		//存储现在遍历到的最大的照片id
			char file_name[50];                 // 用于存储文件名

			file_name_num_id = list_files("/home/img");
			if(file_name_num_id <0 ){
				perror("目录获取异常！");
				return NULL;
			}

			// 使用当前文件编号生成新的文件名，-2 是为了避免“.”和“..”目录项
			sprintf(file_name, "/home/img/%03d.jpg", file_name_num_id-2);	
			
			FILE *file = fopen(file_name, "r");  // 尝试打开文件以检查是否已存在
			while(1)
			{
				 // 如果文件已存在，关闭文件并递增文件名编号，直到找到一个可用的文件名
				if(file != NULL) 
				{
					fclose(file);
					file_name_num_id++;
					sprintf(file_name, "/usr/share/lvgl_user/jpg/%03d.jpg", file_name_num_id-2);	//-2是为了滤除上一级和下一级目录项
					file = fopen(file_name, "r");
				}
				else
				{
					// 如果文件不存在，跳出循环
					break;
				}
			}

			file = fopen(file_name, "w+");	//建立文件用于保存一帧数据
			// 将当前缓冲区的摄像头数据写入文件
			fwrite(mmpaddr[readbuffer.index], readbuffer.length, 1, file);
			fclose(file);     // 完成写入后关闭文件

			// 设置拍照状态为“已准备好”，表示拍照操作完成
			capture_signal = CAPTURE_STA_READY;
		}

		LCD_JPEG_Show((const char *)mmpaddr[readbuffer.index], readbuffer.length); //显示在LCD上
		
		ret = ioctl(fd, VIDIOC_QBUF, &readbuffer); //读取数据后将缓冲区放入队列
		if(ret < 0)
			perror("放入队列失败");
	}

	return NULL;
}

static int Camera_Init(void)
{
	// 1. 打开摄像头设备
	fd = open("/dev/video1", O_RDWR);
	if(fd < 0)
	{
		perror("打开设备失败");
		return -1;
	}

	// 2. 设置摄像头采集格式
	struct v4l2_format vfmt;                 // 定义一个 v4l2_format 结构体来存储摄像头的格式信息
	vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // 设置为视频捕获类型
	vfmt.fmt.pix.width = 352;				// 设置视频的宽度（352 像素）
	vfmt.fmt.pix.height = 288;              // 设置视频的高度（288 像素）
	vfmt.fmt.pix.pixelformat =  V4L2_PIX_FMT_MJPEG; // 设置视频的像素格式为 MJPEG（压缩格式）

	// 使用 ioctl 函数设置摄像头的格式
	int ret = ioctl(fd, VIDIOC_S_FMT, &vfmt); // VIDIOC_S_FMT:设置捕获格式
	if(ret < 0)
	{
		perror("设置采集格式错误");
	}

	// 读取并打印当前的采集格式
	memset(&vfmt, 0, sizeof(vfmt)); // 清空 vfmt 结构体
	vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // 设置为视频捕获类型
	ret = ioctl(fd, VIDIOC_G_FMT, &vfmt);	 // 使用 VIDIOC_G_FMT 获取当前格式
	if(ret < 0)
	{
		perror("读取采集格式失败");
	}

	// 打印采集的分辨率和像素格式信息
	printf("设置分辨率width = %d\n", vfmt.fmt.pix.width);
	printf("设置分辨率height = %d\n", vfmt.fmt.pix.height);
	unsigned char *p = (unsigned char*)&vfmt.fmt.pix.pixelformat;
	printf("pixelformat = %c%c%c%c\n", p[0],p[1],p[2],p[3]);	// 打印像素格式的字符

	// 3. 申请缓冲区
	struct v4l2_requestbuffers reqbuffer; // 定义请求缓冲区的结构体
	reqbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // 设置为视频捕获类型
	reqbuffer.count = 4;	// 申请4个缓冲区
	reqbuffer.memory = V4L2_MEMORY_MMAP;	//采用内存映射的方式

	ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuffer); // 使用 ioctl 请求缓冲区
	if(ret < 0)
	{
		perror("申请缓冲队列失败");
	}
	
	// 4. 映射缓冲区到内存
	struct v4l2_buffer mapbuffer; // 定义缓冲区结构体
	mapbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // 设置缓冲区为视频捕获类型
	for(int i = 0; i < 4; i++) // 循环映射 4 个缓冲区
	{
		mapbuffer.index = i;
		ret = ioctl(fd, VIDIOC_QUERYBUF, &mapbuffer);	//查询缓存信息
		if (ret < 0)  // 如果请求缓冲区失败，输出错误信息
		{
			perror("申请缓冲队列失败");
		}

		// 内存映射操作，将缓冲区映射到应用程序的地址空间
		mmpaddr[i] = (unsigned char *)mmap(NULL, mapbuffer.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, mapbuffer.m.offset);//mapbuffer.m.offset映射文件的偏移量
		if (mmpaddr[i] == MAP_FAILED)  // 如果映射失败，输出错误信息
        {
            perror("映射缓冲区失败");
            return -1;
        }
		
		addr_length[i] = mapbuffer.length; // 存储缓冲区大小
		ret = ioctl(fd, VIDIOC_QBUF, &mapbuffer); // 将缓冲区放入队列
		if(ret < 0)
		{
            perror("将缓冲区加入队列失败");
            return -1;
        }
	}

	// 5. 启动摄像头设备
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd, VIDIOC_STREAMON, &type);
	if(ret < 0)
		perror("打开设备失败");

	/* 创建"摄像头画面读取线程" */
	ret = pthread_create(&tid, NULL, Camera_Read_thread_func, NULL);
	if (ret)
	{
		perror("pthread_create err!");
		return -1;
	}

	ret = pthread_detach(tid);		//进行线程分离，线程结束自动回收资源
	if (ret != 0) {
		// 处理错误
		perror("线程分离失败！");
		return -1;
	}

	return 0;	
}

//拍照事件
static void my_Btn_capture_event_cb(lv_event_t * e)
{
	lv_obj_t * obj_e = lv_event_get_target(e);        // 获取触发事件的部件(对象)
	lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_CLICKED)
	{
		lv_obj_t *Frame_sw = lv_obj_get_child(Camera_page, FRAME_SW_CHILD_ID);	//获取到开关对象的地址

		lv_obj_t * Label_capture = lv_label_create(Camera_page);
		lv_obj_set_style_text_font(Label_capture, ft_info.font, LV_STATE_DEFAULT);
		lv_label_set_recolor(Label_capture, 1);
		lv_obj_align_to(Label_capture, obj_e, LV_ALIGN_CENTER, -24, 100);

		if(lv_obj_has_state(Frame_sw, LV_STATE_CHECKED))	//确保摄像头在打开状态
		{
			//加入拍照程序
			capture_signal = CAPTURE_STA_RUN;
			while(1)
			{
				usleep(5000);
				if(capture_signal == CAPTURE_STA_READY) break;
			}

			lv_label_set_text(Label_capture, "#0EB16C 拍照成功！#");
		}
		else
		{
			lv_label_set_text(Label_capture, "#B20000 拍照失败！\n#B20000 摄像头未打开！#");
		}
		lv_obj_del_delayed(Label_capture, 1500);
	}
}


static void my_Sw_Frame_event_cb(lv_event_t * e)
{
	lv_obj_t * obj_e = lv_event_get_target(e);        // 获取触发事件的部件(对象)
	lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_VALUE_CHANGED)
	{
		lv_obj_t * Label_state = lv_obj_get_child(Camera_page, LABEL_STATE_CHILD_ID);		//获取到画面背景对象的地址

		if(lv_obj_has_state(obj_e, LV_STATE_CHECKED))	//状态“1” ，打开摄像头并显示至LCD
		{
			int ret = Camera_Init();
			if (ret != 0) {
				// 处理错误
				printf("打开摄像头失败！请检查摄像头是否接入！\n");
				lv_label_set_text(Label_state, "#B20000 打开摄像头失败！请检查摄像头是否接入！#");
				lv_obj_clear_state(obj_e, LV_STATE_CHECKED);		//设置为关状态
				return ;
			}

			lv_label_set_text(Label_state, "#0EB16C 摄像头已打开#");

		}
		else	//状态"0"销毁LCD显示线程并关闭摄像头的内存映射
		{
			int ret = pthread_cancel(tid);
			//printf("my_Sw_Frame_event_cb\n");
			if (ret != 0) {
				// 处理错误
				printf("线程销毁失败！\n");
				return ;
			}

			lv_obj_t * Frame_obj = lv_obj_get_child(Camera_page, FRAME_OBJ_CHILD_ID);		//获取到画面背景对象的地址
			lv_obj_set_style_bg_color(Frame_obj, lv_color_hex(0xEDEDF5), LV_STATE_DEFAULT);	//设置画面背景颜色，偏灰
			lv_label_set_text(Label_state, "#519ABA 摄像头未打开#");

			int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			//关闭设备
			ret = ioctl(fd, VIDIOC_STREAMOFF, &type);
			if(ret < 0)
				perror("关闭设备失败");

			for(int i = 0; i < 4; i++){
				//printf("my_Sw_Frame_event_cb test1\n");
				munmap(mmpaddr[i], addr_length[i]);
				//printf("my_Sw_Frame_event_cb test1\n");
			}
				
			close(fd);
			//printf("my_Sw_Frame_event_cb test1\n");
		}
	}
}

static void CameraPageInit(void *User_data)
{ 
	// 创建一个输入设备组，用于组织多个输入设备（如触摸屏、按钮等）
	lv_group_t * g = lv_group_create();
	lv_group_set_default(g);								                                //添加到默认组
	lv_indev_set_group(mouse_indev, g);	                                                    //将输入设备（如鼠标）加入组

	//创建返回按钮	id_0
    lv_obj_t * Btn_return = lv_btn_create(Camera_page);                                     // 在 Camera_page 上创建一个按钮
    lv_obj_set_size(Btn_return, 100, 100);                                                  // 设置按钮大小
	lv_obj_align(Btn_return, LV_ALIGN_TOP_LEFT, 20, 20);                                    // 按钮在屏幕左上角对齐
	lv_obj_t * Label_return = lv_label_create(Btn_return);                                  // 在按钮上创建标签
	FT_font_Set(28);                                                                        // 设置字体大小
	lv_obj_set_style_text_font(Label_return, ft_info.font, LV_STATE_DEFAULT);               // 设置标签字体
	lv_label_set_text(Label_return, "返回");                                                // 设置标签文本
	lv_obj_align(Label_return, LV_ALIGN_CENTER, 0, -4);                                     // 标签居中对齐，稍微向上偏移

	//创建摄像头画面显示的区域	id_1
	lv_obj_t * Frame_obj= lv_obj_create(Camera_page);                                       // 创建一个框架对象
	lv_obj_set_size(Frame_obj, 640, 480);													// 设置显示区域的大小
	lv_obj_set_style_opa(Frame_obj, LV_OPA_100, LV_STATE_DEFAULT);							// 设置透明度
	lv_obj_set_style_bg_color(Frame_obj, lv_color_hex(0xEDEDF5), LV_STATE_DEFAULT);			// 设置背景颜色 偏灰色
	lv_obj_set_style_outline_width(Frame_obj, 2, LV_STATE_DEFAULT);							// 设置边框宽度
	lv_obj_set_style_outline_color(Frame_obj, lv_color_hex(0XBDBDBD), LV_STATE_DEFAULT);	// 设置边框颜色
	lv_obj_set_style_outline_pad(Frame_obj, 2, LV_STATE_DEFAULT);							// 设置画面内边距
	lv_obj_align(Frame_obj, LV_ALIGN_CENTER, 0, 0);											// 对齐到屏幕中心

	//创建打开摄像头的开关	id_2
	lv_obj_t * Frame_sw = lv_switch_create(Camera_page);                                    // 创建一个开关
	lv_obj_set_size(Frame_sw, 80, 40);                                                      // 设置开关的大小
	lv_obj_align_to(Frame_sw, Frame_obj, LV_ALIGN_OUT_TOP_LEFT, 0, -10);                    // 将开关放置在摄像头画面框架的上方

	//设备状态显示文本	id_3
	lv_obj_t * Label_state = lv_label_create(Camera_page);                                  // 创建状态标签
	lv_obj_set_style_text_font(Label_state, ft_info.font, LV_STATE_DEFAULT);                // 设置字体
	lv_label_set_recolor(Label_state, 1);                                                   // 启用颜色重绘（例如设置颜色标记）
	lv_label_set_text(Label_state, "#519ABA 摄像头未打开#");                                 // 设置显示文本
	lv_obj_align_to(Label_state, Frame_sw, LV_ALIGN_CENTER, 160, 0);                        // 将标签对齐到开关旁边
	
	//拍照按钮	id_4
	lv_obj_t * Btn_capture = lv_btn_create(Camera_page);                                    // 在 Camera_page 上创建一个按钮
    lv_obj_set_size(Btn_capture, 120, 60);                                                  // 设置按钮的尺寸为 120x60
	lv_obj_set_style_radius(Btn_capture, 15, LV_STATE_DEFAULT);                             // 设置按钮的圆角半径为 15
	lv_obj_align_to(Btn_capture, Frame_obj, LV_ALIGN_TOP_RIGHT, 170, 0);                    // 将按钮定位到摄像头画面框架的右上角，距右边 170 像素
	lv_obj_t * Label_capture = lv_label_create(Btn_capture);                                // 在按钮上创建一个标签
	lv_obj_set_style_text_font(Label_capture, ft_info.font, LV_STATE_DEFAULT);              // 设置标签字体
	//lv_label_set_text(Label_capture, "点击拍照");
	lv_label_set_text_fmt(Label_capture, "点击拍照");                                        // 使用格式化文本设置标签文本
	lv_obj_align(Label_capture, LV_ALIGN_CENTER, 0, -3);                                    // 将文本居中对齐，略微向上偏移 3 像素

	//注册返回按键触发事件
	lv_obj_add_event_cb(Btn_capture, my_Btn_capture_event_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_event_cb(Btn_return, my_Btn_return_event_cb, LV_EVENT_CLICKED, NULL);	    
	lv_obj_add_event_cb(Frame_sw, my_Sw_Frame_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

	//创建页面时初始化LCD一次
	if(LCD_Init() != 0)
	{	
		perror("初始化LCD失败");
		return ;
	}
}

static void CameraPageCreate()
{
	//创建一个新的对象，NULL 表示该对象没有父对象，因此它将被创建为根对象，或者说是顶级对象
	Camera_page = lv_obj_create(NULL);
}

static void CameraPageRun(void *pParams)
{
	//lv_scr_act()：这个函数返回当前活动的屏幕对象，通常是显示在屏幕上的界面。
	//lv_obj_clean()：这个函数会清除指定对象的所有子对象。在这里，它会清除当前活动屏幕上的所有对象。
	lv_obj_clean(lv_scr_act());

	//页面初始化
	CameraPageInit(NULL);

	//切换当前活动屏幕，加载并显示 Camera_page 作为新的活动屏幕。
    lv_scr_load(Camera_page);
}

static PageAction Camera_Page = {
	.name = "Camera",
	.Create = CameraPageCreate,
	.Run  = CameraPageRun,
};

void Camera_Page_Registered(void)
{
	Registered_Page(&Camera_Page);
}