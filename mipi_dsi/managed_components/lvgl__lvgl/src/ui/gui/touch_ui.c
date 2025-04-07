//
// Created by Administrator on 25-2-21.
//
#include "touch_ui.h"

// void item_click_cb(lv_event_t* e) {
// 	typedef void (*page_func)();
// 	page_func page_list[]={touch_ui_page1,touch_ui_page2,touch_ui_page3,touch_ui_page4,touch_ui_page5};
// 	int index=(intptr_t)lv_event_get_user_data(e);
// 	lv_obj_clean(scr);
// 	page_list[index]();
// 	printf("click item %d\n",index);
// }

//**************SCREENS INIT*****************//
void touch_ui_screen1_init(void);

void touch_ui_screen2_init(void);


//*************CALLBACK FUNCTIONS**************//
void toggle_screen1_event(lv_event_t* e) {
	lv_event_code_t code=lv_event_get_code(e);

	if (code==LV_EVENT_CLICKED) {
		ui_screen_change(&touch_screen1,LV_SCR_LOAD_ANIM_FADE_IN,500,0,&touch_ui_screen1_init);
	}
}
void toggle_screen2_event(lv_event_t* e) {
	lv_event_code_t code=lv_event_get_code(e);

	if (code==LV_EVENT_CLICKED) {
		ui_screen_change(&touch_screen2,LV_SCR_LOAD_ANIM_FADE_IN,500,0,&touch_ui_screen2_init);
	}
}
void back_home_event(lv_event_t* e) {
	lv_event_code_t code=lv_event_get_code(e);
	if (code==LV_EVENT_CLICKED) {
		ui_screen_change(&home_scr,LV_SCR_LOAD_ANIM_FADE_IN,500,0,&touch_ui_home_init);
	}
}
