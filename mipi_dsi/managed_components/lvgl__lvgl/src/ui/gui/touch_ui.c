//
// Created by Administrator on 25-2-21.
//
#include <stdio.h>
#include "time.h"
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
void touch_ui_screen3_init(void);

//*************TIMER CALLBACK FUNCTIONS**************//

void update_localtime_cb(lv_timer_t* timer) {
	const time_t now=time(NULL);
	const struct tm* tm=localtime(&now);
	lv_label_set_text_fmt(label_time,"%02d:%02d:%02d",tm->tm_hour,tm->tm_min,tm->tm_sec);
	lv_label_set_text_fmt(label_date,"%04d-%02d-%02d",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday);
}

//*************CALLBACK FUNCTIONS**************//
void toggle_screen1_event(lv_event_t* e) {
	lv_event_code_t code=lv_event_get_code(e);

	if (code==LV_EVENT_CLICKED) {
		ui_screen_change(&touch_screen1,LV_SCR_LOAD_ANIM_FADE_IN,200,0,&touch_ui_screen1_init);
	}
}
void toggle_screen2_event(lv_event_t* e) {
	lv_event_code_t code=lv_event_get_code(e);

	if (code==LV_EVENT_CLICKED) {
		ui_screen_change(&touch_screen2,LV_SCR_LOAD_ANIM_FADE_IN,200,0,&touch_ui_screen2_init);
	}
}
void toggle_screen3_event(lv_event_t* e) {
	lv_event_code_t code=lv_event_get_code(e);
	if (code==LV_EVENT_CLICKED) {
		ui_screen_change(&touch_screen3,LV_SCR_LOAD_ANIM_FADE_IN,200,0,&touch_ui_screen3_init);
	}
}
void back_home_event(lv_event_t* e) {
	lv_event_code_t code=lv_event_get_code(e);
	if (code==LV_EVENT_CLICKED) {
		ui_screen_change(&home_scr,LV_SCR_LOAD_ANIM_FADE_IN,200,0,&touch_ui_home_init);
	}
}
