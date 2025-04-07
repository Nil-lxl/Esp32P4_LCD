//
// Created by Administrator on 25-3-28.
//

#include "ui_helpers.h"

void ui_screen_change(lv_obj_t **target,lv_scr_load_anim_t fademode,int speed,int delay,void(*target_init)(void)){
	if (*target==NULL) {
		target_init();
	}
	lv_screen_load_anim(*target,fademode,speed,delay,false);
}