//
// Created by Administrator on 25-3-28.
//

#include "../gui/touch_ui.h"

/*********************
 *      EXTERN
 *********************/
lv_obj_t* home_scr;
/**********************
 *		GLOBAL
 **********************/
lv_obj_t* navigation;
lv_obj_t* nav_bar;
lv_obj_t* container;
lv_obj_t* nav_foot;
lv_obj_t* tab[NAV_COUNT];
lv_obj_t* tab_container[NAV_COUNT];
lv_obj_t* nav_item[NAV_COUNT][ITEM_COUNT];
lv_obj_t* item_btn[NAV_COUNT][ITEM_COUNT];
lv_obj_t* item_label[NAV_COUNT][ITEM_COUNT];
lv_style_t bg_style;
lv_grad_dsc_t bg_grad;

lv_obj_t* exit_btn;
lv_obj_t* label_date;
lv_obj_t* label_time;

const char* item_text[NAV_COUNT][ITEM_COUNT] = {
	{"SETTING", "EMAIL", "BenchMark", "Image", "ITEM5","ITEM6","ITEM7","ITEM8"},
	{"Calculator", "ITEM10", "ITEM11", "ITEM12", "ITEM13","ITEM14","ITEM15","ITEM16"},
	{"ITEM17", "ITEM18", "ITEM19", "ITEM20", "ITEM21","ITEM22","ITEM23","ITEM24"},
};
const lv_color_t grad_colors[2]={
	LV_COLOR_MAKE(0x18,0xc6,0xf6),
	LV_COLOR_MAKE(0x33,0xe1,0xa5),
};

const lv_image_dsc_t* img_list[NAV_COUNT][ITEM_COUNT]={
	{&HOME,&EMAIL,&MONITOR,&IMAGE,&SECURE,&HOURS,&CHAT,&CAMERA},
	{&CALCULATOR,&BLUETOOTH,&WIFI,&CALL,&INFO,&WALLET,&CALENDAR,&EMBEDDED},
	{},
};

void touch_ui_home_init(void) {
	show_exit=false;
	home_scr = lv_scr_act();
	navigation=lv_tabview_create(home_scr);
	lv_obj_set_size(navigation,lv_pct(100),lv_pct(100));
	lv_obj_center(navigation);
	lv_tabview_set_tab_bar_size(navigation,0);

	tab[0]=lv_tabview_add_tab(navigation, "tab1");
	tab[1]=lv_tabview_add_tab(navigation, "tab1");
	tab[2]=lv_tabview_add_tab(navigation, "tab1");

	lv_obj_set_style_pad_all(tab[0],0,0);
	lv_obj_set_style_pad_all(tab[1],0,0);
	lv_obj_set_style_pad_all(tab[2],0,0);

	lv_style_init(&bg_style);

	/*	bg color gradient	*/
	lv_gradient_init_stops(&bg_grad,grad_colors,NULL,NULL,sizeof(grad_colors)/sizeof(lv_color_t));
	lv_grad_radial_init(&bg_grad,LV_GRAD_CENTER,LV_GRAD_CENTER,LV_GRAD_RIGHT,LV_GRAD_BOTTOM,LV_GRAD_EXTEND_PAD);
	lv_style_set_bg_grad(&bg_style,&bg_grad);
	lv_obj_add_style(navigation,&bg_style,0);
	// lv_obj_set_style_bg_color(scr,lv_palette_lighten(LV_PALETTE_GREEN,3),0);

	nav_bar=lv_obj_create(home_scr);
	lv_obj_set_size(nav_bar,lv_pct(100),30);
	lv_obj_set_align(nav_bar,LV_ALIGN_TOP_MID);
	lv_obj_set_style_border_width(nav_bar,0,0);
	lv_obj_set_style_radius(nav_bar,0,0);
	lv_obj_set_style_bg_opa(nav_bar,LV_OPA_40,0);
	lv_obj_set_style_pad_all(nav_bar,0,0);
	lv_obj_clear_flag(nav_bar,LV_OBJ_FLAG_SCROLLABLE);

	exit_btn=lv_imgbtn_create(lv_layer_top());
	lv_obj_align(exit_btn,LV_ALIGN_OUT_LEFT_TOP,5,5);
	lv_obj_set_size(exit_btn,25,25);
	lv_imgbtn_set_src(exit_btn,LV_IMAGEBUTTON_STATE_RELEASED,&EXIT_SMALL,NULL,NULL);
	lv_obj_add_flag(exit_btn,LV_OBJ_FLAG_HIDDEN);

	label_time=lv_label_create(nav_bar);
	lv_obj_align(label_time,LV_ALIGN_RIGHT_MID,0,0);
	lv_label_set_text_fmt(label_time,"00:00:00");
	label_date=lv_label_create(nav_bar);
	lv_obj_align(label_date,LV_ALIGN_CENTER,lv_pct(20),0);
	lv_label_set_text_fmt(label_date,"xxxx-xx-xx");

	/*********************
	 *      TAB1
	 *********************/
	tab_container[0] = lv_obj_create(tab[0]);
	lv_obj_set_size(tab_container[0],CONTAINER_WIDTH,CONTAINER_HEIGHT);
	lv_obj_set_align(tab_container[0],LV_ALIGN_CENTER);
	/*	NAVIGATION FLEX LAYOUT SETTING	*/
	lv_obj_set_flex_flow(tab_container[0],LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_flex_align(tab_container[0],LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_SPACE_EVENLY);
	lv_obj_set_style_pad_column(tab_container[0],NAV_ITEM_MARGIN,0);
	// lv_obj_set_style_bg_color(tab_container[0],lv_palette_lighten(LV_PALETTE_BLUE,4),0);
	lv_obj_set_style_bg_opa(tab_container[0],0,0); //set bg transparent
	lv_obj_set_style_border_width(tab_container[0],0,0);
	// lv_obj_set_scroll_snap_x(tab_container[0],LV_SCROLL_SNAP_CENTER);//设置控件捕捉
	// lv_obj_set_scrollbar_mode(tab_container[0],LV_SCROLLBAR_MODE_OFF);//隐藏滚动条
	lv_obj_clear_flag(tab_container[0],LV_OBJ_FLAG_SCROLLABLE);

	/*********************
	*      TAB2
	*********************/
	tab_container[1] = lv_obj_create(tab[1]);
	lv_obj_set_size(tab_container[1],CONTAINER_WIDTH,CONTAINER_HEIGHT);
	lv_obj_set_align(tab_container[1],LV_ALIGN_CENTER);
	/* NAVIGATION FLEX LAYOUT SETTING*/
	lv_obj_set_flex_flow(tab_container[1],LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_flex_align(tab_container[1],LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_SPACE_EVENLY);
	lv_obj_set_style_pad_column(tab_container[1],NAV_ITEM_MARGIN,0);
	lv_obj_set_style_bg_opa(tab_container[1],0,0); //set bg transparent
	lv_obj_set_style_border_width(tab_container[1],0,0);
	lv_obj_clear_flag(tab_container[1],LV_OBJ_FLAG_SCROLLABLE);

	/*********************
	*      TAB3
	*********************/
	tab_container[2] = lv_obj_create(tab[2]);
	lv_obj_set_size(tab_container[2],CONTAINER_WIDTH,CONTAINER_HEIGHT);
	lv_obj_set_align(tab_container[2],LV_ALIGN_CENTER);
	/* NAVIGATION FLEX LAYOUT SETTING*/
	lv_obj_set_flex_flow(tab_container[2],LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_flex_align(tab_container[2],LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_SPACE_EVENLY);
	lv_obj_set_style_pad_column(tab_container[2],NAV_ITEM_MARGIN,0);
	lv_obj_set_style_bg_opa(tab_container[2],0,0); //set bg transparent
	lv_obj_set_style_border_width(tab_container[2],0,0);
	lv_obj_clear_flag(tab_container[2],LV_OBJ_FLAG_SCROLLABLE);

	//Application Buttons
	for (int i=0;i<NAV_COUNT;i++) {
		for (int j=0;j<ITEM_COUNT;j++) {
#if UI_EXAMPLE_USE_IMGBTN
			nav_item[i][j]=lv_obj_create(tab_container[i]);
			item_btn[i][j]=lv_imgbtn_create(nav_item[i][j]);
			item_label[i][j]=lv_label_create(nav_item[i][j]);
			lv_img_dsc_t* img=img_list[i][j];
			lv_imgbtn_set_src(item_btn[i][j],LV_IMGBTN_STATE_RELEASED,img,NULL,NULL);
#else
			/*	create item objects in navigation container	*/
			nav_item[i][j]=lv_obj_create(tab_container[i]);
			item_btn[i][j]=lv_button_create(nav_item[i][j]);
			item_label[i][j]=lv_label_create(nav_item[i][j]);
#endif
			/* clear default styles of item obj	*/
			lv_obj_set_size(nav_item[i][j],NAV_ITEM_WIDTH,NAV_ITEM_HEIGHT);
			lv_obj_set_style_bg_opa(nav_item[i][j],LV_OPA_0,0);
			lv_obj_set_style_border_width(nav_item[i][j],0,0);
			lv_obj_set_style_radius(nav_item[i][j],0,0);
			lv_obj_set_style_pad_all(nav_item[i][j],0,0);

			lv_obj_set_size(item_btn[i][j],NAV_ITEM_WIDTH,NAV_ITEM_WIDTH);

			lv_obj_set_align(item_label[i][j],LV_ALIGN_BOTTOM_MID);
			lv_obj_set_width(item_label[i][j],NAV_ITEM_WIDTH);
			lv_label_set_text_fmt(item_label[i][j],item_text[i][j]);
			/* set text align in label*/
			lv_obj_set_style_text_align(item_label[i][j],LV_TEXT_ALIGN_CENTER,0);
			/* set label long text mode*/
			lv_label_set_long_mode(item_label[i][j],LV_LABEL_LONG_SCROLL_CIRCULAR);
		}
	}
	/* add event callbacks when togging screen 	*/
	lv_obj_add_event_cb(exit_btn,back_home_event,LV_EVENT_CLICKED,NULL);

	lv_obj_add_event_cb(item_btn[0][0],toggle_screen1_event,LV_EVENT_ALL,NULL);
	lv_obj_add_event_cb(item_btn[0][1],toggle_screen2_event,LV_EVENT_ALL,NULL);
	lv_obj_add_event_cb(item_btn[0][2],toggle_screen3_event,LV_EVENT_ALL,NULL);
	lv_obj_add_event_cb(item_btn[0][3],toggle_image_event,LV_EVENT_ALL,NULL);
	lv_obj_add_event_cb(item_btn[1][0],toggle_calculator_event,LV_EVENT_ALL,NULL);
	lv_timer_create(update_localtime_cb,1000,NULL);

}
