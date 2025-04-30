//
// Created by Administrator on 25-4-23.
//
#include "../gui/touch_ui.h"

lv_obj_t* calculator_screen;
void calculator_screen_init(void) {
	calculator_screen=lv_obj_create(NULL);
	lv_obj_set_style_bg_color(calculator_screen,lv_palette_darken(LV_PALETTE_BLUE,2),0);

}