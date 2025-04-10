#include <page_manager.h>
#include <stdio.h>
#include <ui.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

LV_FONT_DECLARE(my_t02);
#define  FONTAWESOME_SYMBOL_image     		"\xef\x80\xbe"
#define  FONTAWESOME_SYMBOL_house     		"\xef\x80\x95"
#define  FONTAWESOME_SYMBOL_phone     		"\xef\x82\x95"
#define  FONTAWESOME_SYMBOL_envelope     	"\xef\x83\xa0"
#define  FONTAWESOME_SYMBOL_camera_retro    "\xef\x82\x83"
#define  FONTAWESOME_SYMBOL_comment     	"\xef\x81\xb5"
#define  FONTAWESOME_SYMBOL_film     		"\xef\x80\x88"
#define  FONTAWESOME_SYMBOL_wifi     		"\xef\x87\xab"
#define  FONTAWESOME_SYMBOL_music     		"\xef\x80\x81"
#define  FONTAWESOME_SYMBOL_twitter     	"\xef\x82\x99"

static lv_obj_t * main_page = NULL;

/**********************************按键回调函数**************************************/
static void my_Camera_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_CLICKED)
	{
		Page("Camera")->Run(NULL);
	}
}

static void my_Album_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_CLICKED)
	{
		Page("album")->Run(NULL);
	}
}

static void my_Sr04_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_CLICKED)
	{
		Page("Sr04")->Run(NULL);
	}
}

static void my_At24_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_CLICKED)
	{
		Page("At24c02")->Run(NULL);
	}
}

static void my_DHT11_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_CLICKED)
	{
		Page("DHT11")->Run(NULL);
	}
}

static void my_Sg90_event_cb(lv_event_t * e)
{
    lv_event_code_t code_e = lv_event_get_code(e);    // 获取当前部件(对象)触发的事件代码

	if(code_e == LV_EVENT_CLICKED)
	{
		Page("sg90")->Run(NULL);
	}
}

/**********************************主页面设计**************************************/
static void MainPageCreate(void)
{
	//创建主界面
	// main_page = lv_scr_act();
	main_page = lv_obj_create(NULL);
}

static void MainPageInit(void *User_data)
{
    LV_IMG_DECLARE(scx_bg);
	lv_obj_t * img = lv_img_create(main_page);
	lv_img_set_src(img, &scx_bg);

    //创建组
	lv_group_t * g = lv_group_create();
	lv_group_set_default(g);			//添加到默认组
	lv_indev_set_group(mouse_indev, g);	//输入设备添加进组

	/*创建按钮样式*/
    static lv_style_t style;         	//创建样式
	lv_style_init(&style);           	//初始化样式
	lv_style_set_radius(&style,8);   	//设置样式的圆角
	lv_style_set_width(&style,80);  	//设置样式的宽度
	lv_style_set_height(&style, 80); 	//设置样式的高度
	lv_style_set_pad_all(&style, 3);
    // lv_style_set_align(&style, LV_ALIGN_CENTER);
	lv_style_set_bg_color(&style, lv_color_hex(0x009688));
	lv_style_set_text_color(&style, lv_color_hex(0xffffff));

	/*创建图标样式*/
    static lv_style_t style_image;         							//创建样式
	lv_style_init(&style_image);           							//初始化图标样式
	lv_style_set_text_font(&style_image, &my_t02);					//初始化图标样式字体
	lv_style_set_text_color(&style_image, lv_color_hex(0xffffff));	//初始化图标样式颜色，白色

	/*******************************************第一个APP，摄像头 ****************************************/ 
    lv_obj_t * Btn_Camera = lv_btn_create(main_page);
	lv_obj_align(Btn_Camera, LV_ALIGN_CENTER, -360,-100);
	lv_obj_add_style(Btn_Camera,&style,LV_STATE_DEFAULT);

    lv_obj_t * Label_Camera = lv_label_create(main_page);				//添加APP名称
	lv_obj_set_style_text_color(Label_Camera, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
	lv_label_set_recolor(Label_Camera, 1);
	lv_label_set_text_fmt(Label_Camera, "#0000ff 摄像头#");							//添加名字
	lv_obj_set_style_text_font(Label_Camera, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_Camera, Btn_Camera, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方
	
    lv_obj_t * Label_Camera_str = lv_label_create(main_page);			// 使用新添加的图标（symbol）
    lv_label_set_text(Label_Camera_str, FONTAWESOME_SYMBOL_camera_retro);			//添加图标
	lv_obj_add_style(Label_Camera_str,&style_image,LV_STATE_DEFAULT);		//添加样式，白色图标以及字体设置
	lv_obj_align_to(Label_Camera_str, Btn_Camera, LV_ALIGN_CENTER, 0, 0);			//居中

	/*******************************************第二个APP，简易的相册 ****************************************/ 
	lv_obj_t * Btn_album = lv_btn_create(main_page);
	lv_obj_align(Btn_album, LV_ALIGN_CENTER, -180,-100);
	//lv_obj_align_to(Btn_album, Btn_Camera, LV_ALIGN_CENTER, 180, 0);
	lv_obj_add_style(Btn_album,&style,LV_STATE_DEFAULT);

	lv_obj_t * Label_album = lv_label_create(main_page);			// 使用新添加的图标（symbol）
    lv_label_set_text(Label_album, FONTAWESOME_SYMBOL_image);			//添加图标
	lv_obj_add_style(Label_album,&style_image,LV_STATE_DEFAULT);		//添加样式，白色图标以及字体设置
	lv_obj_align_to(Label_album, Btn_album, LV_ALIGN_CENTER, 0, 0);			//居中

	lv_obj_t * Label_album_str = lv_label_create(main_page);				//添加APP名称
	lv_obj_set_style_text_color(Label_album_str, lv_color_hex(0xffffff), LV_STATE_DEFAULT);
	lv_label_set_recolor(Label_album_str, 1);
	lv_label_set_text_fmt(Label_album_str, "#0000ff 相册#");							//添加名字
	lv_obj_set_style_text_font(Label_album_str, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_album_str, Btn_album, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方

	/*******************************************第三个APP，超声波 ****************************************/ 
	lv_obj_t * Btn_Sr04 = lv_btn_create(main_page);
	lv_obj_add_style(Btn_Sr04,&style,LV_STATE_DEFAULT);
	//lv_obj_align(Btn_Sr04, LV_ALIGN_CENTER, 360,-100);
	lv_obj_align_to(Btn_Sr04, Btn_album, LV_ALIGN_CENTER, 180, 0);
	lv_obj_t * Label_Sr04 = lv_label_create(main_page);
    lv_label_set_text(Label_Sr04, FONTAWESOME_SYMBOL_house);
	lv_obj_add_style(Label_Sr04,&style_image,LV_STATE_DEFAULT);
	lv_obj_align_to(Label_Sr04, Btn_Sr04, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t * Label_Sr04_str = lv_label_create(main_page);				//添加APP名称 
	lv_label_set_recolor(Label_Sr04_str, 1);
	lv_label_set_text(Label_Sr04_str, "#0000ff 超声波#");						//添加名字
	lv_obj_set_style_text_font(Label_Sr04_str, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_Sr04_str, Btn_Sr04, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方

	/*******************************************第四个APP，I2C设备 ****************************************/ 
	lv_obj_t * Btn_At24 = lv_btn_create(main_page);
	lv_obj_add_style(Btn_At24,&style,LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_At24, Btn_Sr04, LV_ALIGN_CENTER, 180, 0);
	lv_obj_t * Label_At24 = lv_label_create(main_page);
    lv_label_set_text(Label_At24, FONTAWESOME_SYMBOL_phone);
	lv_obj_add_style(Label_At24,&style_image,LV_STATE_DEFAULT);
	lv_obj_align_to(Label_At24, Btn_At24, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t * Label_AT24_str = lv_label_create(main_page);				//添加APP名称
	lv_label_set_recolor(Label_AT24_str, 1);
	lv_label_set_text(Label_AT24_str, "#0000ff AT24C02#");						//添加名字
	lv_obj_set_style_text_font(Label_AT24_str, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_AT24_str, Btn_At24, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方

	/*******************************************第五个APP，温湿度 ****************************************/ 
	lv_obj_t * Btn_DHT11 = lv_btn_create(main_page);
	lv_obj_add_style(Btn_DHT11,&style,LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_DHT11, Btn_At24, LV_ALIGN_CENTER, 180, 0);
	lv_obj_t * Label_DHT11 = lv_label_create(main_page);
    lv_label_set_text(Label_DHT11, FONTAWESOME_SYMBOL_envelope);
	lv_obj_add_style(Label_DHT11,&style_image,LV_STATE_DEFAULT);
	lv_obj_align_to(Label_DHT11, Btn_DHT11, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t * Label_DHT11_str = lv_label_create(main_page);				//添加APP名称
	lv_label_set_recolor(Label_DHT11_str, 1);
	lv_label_set_text(Label_DHT11_str, "#0000ff DHT11温湿度#");						//添加名字
	lv_obj_set_style_text_font(Label_DHT11_str, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_DHT11_str, Btn_DHT11, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方

	/*******************************************第六个APP，SG90舵机 ****************************************/ 
	lv_obj_t * Btn_Sg90 = lv_btn_create(main_page);
	lv_obj_add_style(Btn_Sg90,&style,LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_Sg90, Btn_Camera, LV_ALIGN_CENTER, 0, 200);
	lv_obj_t * Label_Sg90 = lv_label_create(main_page);
    lv_label_set_text(Label_Sg90, FONTAWESOME_SYMBOL_film);
	lv_obj_add_style(Label_Sg90,&style_image,LV_STATE_DEFAULT);
	lv_obj_align_to(Label_Sg90, Btn_Sg90, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t * Label_Sg90_str = lv_label_create(main_page);				//添加APP名称
	lv_label_set_recolor(Label_Sg90_str, 1);
	lv_label_set_text(Label_Sg90_str, "#0000ff SG90舵机#");						//添加名字
	lv_obj_set_style_text_font(Label_Sg90_str, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_Sg90_str, Btn_Sg90, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方

	/*******************************************第七个APP，wifi ****************************************/ 
	lv_obj_t * Btn_wifi = lv_btn_create(main_page);
	lv_obj_add_style(Btn_wifi,&style,LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_wifi, Btn_Sg90, LV_ALIGN_CENTER, 180,0);
	lv_obj_t * Label_wifi = lv_label_create(main_page);
    lv_label_set_text(Label_wifi, FONTAWESOME_SYMBOL_wifi);
	lv_obj_add_style(Label_wifi,&style_image,LV_STATE_DEFAULT);
	lv_obj_align_to(Label_wifi, Btn_wifi, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t * Label_wifi_str = lv_label_create(main_page);				//添加APP名称
	lv_label_set_recolor(Label_wifi_str, 1);
	lv_label_set_text(Label_wifi_str, "#0000ff wifi#");						//添加名字
	lv_obj_set_style_text_font(Label_wifi_str, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_wifi_str, Btn_wifi, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方

	/*******************************************第八个APP，信息 ****************************************/ 
	lv_obj_t * Btn_comment = lv_btn_create(main_page);
	lv_obj_add_style(Btn_comment,&style,LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_comment, Btn_wifi, LV_ALIGN_CENTER,  180,0);
	lv_obj_t * Label_comment = lv_label_create(main_page);
    lv_label_set_text(Label_comment, FONTAWESOME_SYMBOL_comment);
	lv_obj_add_style(Label_comment,&style_image,LV_STATE_DEFAULT);
	lv_obj_align_to(Label_comment, Btn_comment, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t * Label_comment_str = lv_label_create(main_page);				//添加APP名称
	lv_label_set_recolor(Label_comment_str, 1);
	lv_label_set_text(Label_comment_str, "#0000ff 信息#");						//添加名字
	lv_obj_set_style_text_font(Label_comment_str, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_comment_str, Btn_comment, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方

	/*******************************************第九个APP，推特 ****************************************/ 
	lv_obj_t * Btn_twitter = lv_btn_create(main_page);
	lv_obj_add_style(Btn_twitter,&style,LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_twitter, Btn_comment, LV_ALIGN_CENTER, 180,0);
	lv_obj_t * Label_twitter = lv_label_create(main_page);
    lv_label_set_text(Label_twitter, FONTAWESOME_SYMBOL_twitter);
	lv_obj_add_style(Label_twitter,&style_image,LV_STATE_DEFAULT);
	lv_obj_align_to(Label_twitter, Btn_twitter, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t * Label_twitter_str = lv_label_create(main_page);				//添加APP名称
	lv_label_set_recolor(Label_twitter_str, 1);
	lv_label_set_text(Label_twitter_str, "#0000ff twitter#");						//添加名字
	lv_obj_set_style_text_font(Label_twitter_str, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_twitter_str, Btn_twitter, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方

	/*******************************************第十个APP，音乐 ****************************************/ 
	lv_obj_t * Btn_music = lv_btn_create(main_page);
	lv_obj_add_style(Btn_music,&style,LV_STATE_DEFAULT);
	lv_obj_align_to(Btn_music, Btn_twitter, LV_ALIGN_CENTER, 180,0);
	lv_obj_t * Label_music = lv_label_create(main_page);
    lv_label_set_text(Label_music, FONTAWESOME_SYMBOL_music);
	lv_obj_add_style(Label_music,&style_image,LV_STATE_DEFAULT);
	lv_obj_align_to(Label_music, Btn_music, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t * Label_music_str = lv_label_create(main_page);				//添加APP名称
	lv_label_set_recolor(Label_music_str, 1);
	lv_label_set_text(Label_music_str, "#0000ff 音乐#");						//添加名字
	lv_obj_set_style_text_font(Label_music_str, ft_info.font, LV_STATE_DEFAULT);
	lv_obj_align_to(Label_music_str, Btn_music, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);	//移动位置到图标下方


	/*给案件对象添加点击属性  my_Album_event_cb 回调函数 当被点击时 会触发*/
	lv_obj_add_event_cb(Btn_Camera, my_Camera_event_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_event_cb(Btn_album, my_Album_event_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_event_cb(Btn_Sr04, my_Sr04_event_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_event_cb(Btn_At24, my_At24_event_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_event_cb(Btn_DHT11, my_DHT11_event_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_event_cb(Btn_Sg90, my_Sg90_event_cb, LV_EVENT_CLICKED, NULL);

}

static void MainPageRun(void *pParams)
{
    // 第一步 清除上一个界面的所有内容
	lv_obj_clean(lv_scr_act());
    // 第二步 创建主界面下的各种控件
	MainPageInit(NULL);
    // 第三步 设置主界面为要显示的活动屏幕
	lv_scr_load(main_page);
}

static PageAction Main_Page = {
	.name = "main",
	.Create = MainPageCreate,
	.Run  = MainPageRun,
};

//这个接口是用来给page——manager调用的 因此 要用一个外部函数声明
void Main_Page_Registered(void)
{
	Registered_Page(&Main_Page);
}


