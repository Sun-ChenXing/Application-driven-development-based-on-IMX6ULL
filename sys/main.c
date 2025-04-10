#include<stdio.h>
#include<ui.h>
#include<page_manager.h>

int main(){

    // extern void test(void);
    // test();

    //UI_Init();
    PageSystemInit();

    Page("main")->Run(NULL);

    while(1){
        lv_timer_handler();
        usleep(5000);
    }
    return 0;
}