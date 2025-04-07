//
// Created by Administrator on 25-3-28.
//
#include "../gui/touch_ui.h"

lv_obj_t* home_scr;
lv_obj_t* container;
lv_obj_t* menu_item[ITEM_COUNT];
lv_style_t bg_style;
lv_grad_dsc_t bg_grad;
const char* item_text[ITEM_COUNT] = {"ITEM1", "ITEM2", "ITEM3", "ITEM4", "ITEM5"};
const lv_color_t grad_colors[2]={
	LV_COLOR_MAKE(0x18,0xc6,0xf6),
	LV_COLOR_MAKE(0x33,0xe1,0xa5),
};

const lv_image_dsc_t* img_list[]={&home,&email,&notice,&position,&team,&poweroff};

void touch_ui_home_init(void) {
	home_scr = lv_scr_act();
	lv_style_init(&bg_style);

	/*	bg color gradient	*/
	lv_gradient_init_stops(&bg_grad,grad_colors,NULL,NULL,sizeof(grad_colors)/sizeof(lv_color_t));
	lv_grad_radial_init(&bg_grad,LV_GRAD_CENTER,LV_GRAD_CENTER,LV_GRAD_RIGHT,LV_GRAD_BOTTOM,LV_GRAD_EXTEND_PAD);
	lv_style_set_bg_grad(&bg_style,&bg_grad);
	lv_obj_add_style(home_scr,&bg_style,0);
	// lv_obj_set_style_bg_color(scr,lv_palette_lighten(LV_PALETTE_GREEN,3),0);

	container=lv_obj_create(home_scr);
	lv_obj_set_size(container,CONTAINER_WIDTH,CONTAINER_HEIGHT);
	lv_obj_set_align(container,LV_ALIGN_CENTER);
	// lv_obj_set_style_bg_color(container,lv_palette_lighten(LV_PALETTE_BLUE,4),0);
	lv_obj_set_style_bg_opa(container,0,0); //set bg transparent
	lv_obj_set_style_border_width(container,0,0);
	lv_obj_set_scroll_snap_x(container,LV_SCROLL_SNAP_CENTER);//设置控件捕捉
	lv_obj_set_scrollbar_mode(container,LV_SCROLLBAR_MODE_OFF);//隐藏滚动条

	//Application Buttons
	for (int i=0;i<ITEM_COUNT;i++) {
#if UI_EXAMPLE_USE_IMGBTN
		menu_item[i]=lv_imgbtn_create(container);
		lv_img_dsc_t* img=img_list[i];
		lv_imgbtn_set_src(menu_item[i],LV_IMGBTN_STATE_RELEASED,img,NULL,NULL);
#else
		menu_item[i]=lv_btn_create(container);
#endif
		lv_obj_set_size(menu_item[i],MENU_ITEM_SIDE,MENU_ITEM_SIDE);
		lv_obj_set_align(menu_item[i],LV_ALIGN_LEFT_MID);
		lv_obj_set_pos(menu_item[i],i*MENU_ITEM_MARGIN,0);
		// lv_obj_add_event_cb(menu_item[i],item_click_cb,LV_EVENT_CLICKED,(void*)(intptr_t)i);
	}
	lv_obj_scroll_to_view(menu_item[0],LV_ANIM_OFF);

	lv_obj_add_event_cb(menu_item[0],toggle_screen1_event,LV_EVENT_ALL,NULL);
	lv_obj_add_event_cb(menu_item[1],toggle_screen2_event,LV_EVENT_ALL,NULL);
}