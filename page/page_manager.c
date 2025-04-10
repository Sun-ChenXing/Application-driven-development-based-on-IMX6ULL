#include <page_manager.h>
#include <string.h>

// 界面链表头
static PPageAction g_ptPages = NULL;

// 向界面管理程序注册一个界面
void Registered_Page(PPageAction ptPageAction)
{
	// 调用界面对象的创建函数，一般情况下是做一些内存的申请
	ptPageAction->Create();

	// 将新注册的界面对象，利用头插法插入界面链表
	ptPageAction->ptNext = g_ptPages;
	g_ptPages = ptPageAction;
}

// 通过此API找到界面链表中对应名字的界面，并返回其对象地址
PPageAction Page(char *name)
{
	PPageAction ptTmp = g_ptPages;

	while (ptTmp)
	{
		if (strcmp(name, ptTmp->name) == 0)
			return ptTmp;
		ptTmp = ptTmp->ptNext;
	}

	return NULL;
}

// 界面管理程序初始化
void PageSystemInit(void)
{
	//LVGL图形库初始化
	UI_Init();

	extern void Main_Page_Registered(void);
	Main_Page_Registered();

	extern void Camera_Page_Registered(void);
	Camera_Page_Registered();

	extern void Album_Page_Registered(void);
	Album_Page_Registered();

	extern void Sr04_Page_Registered(void);
	Sr04_Page_Registered();

	extern void At24_Page_Registered(void);
	At24_Page_Registered();

	extern void DHT11_page_Registered(void);
	DHT11_page_Registered();

	extern void Sg90_page_Registered(void);
	Sg90_page_Registered();
}

