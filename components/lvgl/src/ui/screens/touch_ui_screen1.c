//
// Created by Administrator on 25-3-28.
//
#include "../gui/touch_ui.h"

lv_obj_t* touch_screen1;
lv_obj_t* scr1_container;

void ui_screen1_init(void) {
	touch_screen1=lv_obj_create(NULL);
	lv_obj_set_style_bg_color(touch_screen1,lv_palette_lighten(LV_PALETTE_CYAN,3),0);
	lv_obj_set_style_bg_opa(touch_screen1,255,0);

	scr1_container=lv_obj_create(touch_screen1);

}