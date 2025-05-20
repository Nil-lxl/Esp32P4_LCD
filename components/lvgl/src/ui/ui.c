//
// Created by Administrator on 25-3-27.
//
#include "lvgl.h"
#include "ui.h"

#define UI_EXAMPLE_USE_ENCODER 0

#if UI_EXAMPLE_USE_ENCODER
#include "gui/encoder_ui.h"
#else
#include "gui/touch_ui.h"
#endif

void UI_init(){
	// lv_obj_t* scr=lv_scr_act();
 //    lv_obj_set_style_bg_color(scr,lv_palette_main(LV_PALETTE_GREY),0);

#if UI_EXAMPLE_USE_ENCODER
	encoder_ui_init();
#else
	touch_ui_home_init();
#endif

}
