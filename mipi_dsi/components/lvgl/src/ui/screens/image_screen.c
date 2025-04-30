//
// Created by Administrator on 25-4-27.
//
#include "../gui/touch_ui.h"

lv_obj_t *image_screen;
lv_obj_t *image_timer;
int image_seq=0;

void image_timer_cb(lv_timer_t* timer) {
	if (image_seq>3)
		image_seq=0;
	else if (image_seq<0)
		image_seq=3;
	switch (image_seq) {
		case 1:
			lv_obj_set_style_bg_image_src(image_screen,&bg_1,0);
			break;
		case 2:
			lv_obj_set_style_bg_image_src(image_screen,&bg_2,0);
			break;
		case 3:
			lv_obj_set_style_bg_image_src(image_screen,&bg_4,0);
			break;
		case 0:
			lv_obj_set_style_bg_image_src(image_screen,&car,0);
			break;
		default:
			break;;
	}
}
void bg_toggle_cb(lv_event_t* e) {
	lv_event_code_t code=lv_event_get_code(e);
	if (code==LV_EVENT_GESTURE) {
		lv_dir_t direction=lv_indev_get_gesture_dir(lv_indev_get_act());
		if (direction==LV_DIR_LEFT) {
			image_seq--;
			// lv_obj_set_style_bg_image_src(image_screen,&bg_1,0);
		}else if (direction==LV_DIR_RIGHT) {
			image_seq++;
		}
	}
}
void image_screen_init(void) {
	lv_timer_create(image_timer_cb,1000,NULL);
	image_screen=lv_obj_create(NULL);

	lv_obj_set_style_bg_image_src(image_screen,&car,0);

	lv_obj_add_event_cb(image_screen,bg_toggle_cb,LV_EVENT_GESTURE,NULL);
}