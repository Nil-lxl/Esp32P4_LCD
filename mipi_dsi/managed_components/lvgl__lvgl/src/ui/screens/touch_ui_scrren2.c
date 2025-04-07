//
// Created by Administrator on 25-3-28.
//
#include "../gui/touch_ui.h"

lv_obj_t* touch_screen2;

void touch_ui_screen2_init(void){
	touch_screen2=lv_obj_create(NULL);

	lv_obj_t* title;
	title=lv_label_create(touch_screen2);

	lv_obj_t* scr2_container;
	scr2_container=lv_obj_create(touch_screen2);
	lv_obj_set_size(scr2_container,CONTAINER_WIDTH,CONTAINER_HEIGHT);
	lv_obj_align(scr2_container,LV_ALIGN_CENTER,0,0);
	lv_obj_set_style_border_width(scr2_container,0,0);

	lv_obj_t* setting_item[5];
	const char* setting_text[5]={"Setting1","Setting2","Setting3","Setting4","Setting5"};
	for (int i=0;i<5;i++) {
		setting_item[i]=lv_btn_create(scr2_container);
		lv_obj_set_size(setting_item[i],200,40);
		lv_obj_set_align(setting_item[i],LV_ALIGN_TOP_MID);
		lv_obj_set_pos(setting_item[i],0,i*40);
		lv_obj_set_style_border_width(setting_item[i],1,0);
		lv_obj_set_style_border_color(setting_item[i],lv_palette_lighten(LV_PALETTE_GREY,2),0);
		lv_obj_set_style_radius(setting_item[i],0,0);
		lv_obj_set_style_bg_color(setting_item[i],lv_color_white(),0);

		lv_obj_t* setting_text_lbl=lv_label_create(setting_item[i]);
		lv_obj_center(setting_text_lbl);
		lv_label_set_text(setting_text_lbl,setting_text[i]);
	}

	lv_obj_t* back_btn=lv_btn_create(scr2_container);
	lv_obj_add_event(back_btn,back_home_event,LV_EVENT_ALL,NULL);
}