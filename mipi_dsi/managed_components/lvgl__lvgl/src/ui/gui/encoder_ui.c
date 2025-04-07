//
// Created by Administrator on 24-12-17.
//
#include "encoder_ui.h"
#include <stdio.h>
#include <unistd.h>
#include "lvgl.h"


static void btn_up_event(lv_event_t* e){
    sum--;
}

static void btn_down_event(lv_event_t* e){
    sum++;
}
static void btn_click_event(lv_event_t* e){

    typedef void (*page_func)();
    page_func page_func_list[]={page1,page2,page3,page4,page5,page6};
    if (page_level==MENU_PAGE) {
        if (lv_obj_has_state(menu_item[menu_seq],LV_STATE_CHECKED)) {
            lv_timer_pause(timer);
            lv_obj_clean(scr);
            page_func_list[menu_seq]();
            page_timer=lv_timer_create(timer_page_cb, 100, NULL);
            // printf("click menu item %d\n",menu_seq+1);
        }
        page_level=CONTENT_PAGE;
    }else if (page_level==CONTENT_PAGE) {
        if (lv_obj_has_state(backBtn,LV_STATE_CHECKED)) {
            lv_timer_pause(page_timer);
            lv_obj_clean(scr);
            encoder_ui_init();
            lv_timer_resume(timer);
        }
    }
}
static int prev_seq;
void timer_cb(lv_timer_t* timer) {

    if (sum!=prev_sum) {
        if (sum>prev_sum) {
            menu_seq++;
        }else if (sum<prev_sum) {
            menu_seq--;
        }

        if (menu_seq>MENU_ITEM_COUNT-1) {
            menu_seq=0;
        }else if (menu_seq<0) {
            menu_seq=MENU_ITEM_COUNT-1;
        }
        prev_sum=sum;
    }
    lv_obj_scroll_to_view(menu_item[menu_seq], LV_ANIM_ON);
    lv_obj_add_state(menu_item[menu_seq],LV_STATE_CHECKED);

    if (menu_seq!=prev_seq) {
        lv_obj_clear_state(menu_item[prev_seq],LV_STATE_CHECKED);
        prev_seq=menu_seq;
    }
    // printf("sum=%d,menu_seq=%d\n", sum,menu_seq);
}
void lv_menu() {

#if USE_IMG_MENU
    lv_img_menu_init();
#else
    encoder_ui_init();
#endif
    timer=lv_timer_create(timer_cb, 100, NULL);
}
void encoder_ui_init(void) {
    page_level=MENU_PAGE;

    scr = lv_scr_act();
    lv_obj_set_size(scr,lv_pct(100),lv_pct(100));
    lv_obj_set_style_bg_color(scr, lv_palette_lighten(LV_PALETTE_PURPLE,5), 0);

    /**********************
     *      构造控件
    **********************/

    container = lv_obj_create(scr);
    lv_obj_set_size(container,MENU_CONTAINER_WIDHT,MENU_CONTAINER_HEIGHT);
    lv_obj_set_align(container,LV_ALIGN_CENTER);
    lv_obj_set_style_bg_color(container,lv_palette_lighten(LV_PALETTE_LIGHT_GREEN,1),0);
    lv_obj_set_style_border_width(container,0,0);

    lv_obj_set_scroll_snap_y(container , LV_SCROLL_SNAP_CENTER );//设置控件捕捉
    lv_obj_set_scrollbar_mode(container , LV_SCROLLBAR_MODE_OFF);//隐藏滚动条

    for (int i=0;i<MENU_ITEM_COUNT;i++) {
        menu_item[i]=lv_btn_create(container);
        lv_obj_set_size(menu_item[i],MENU_ITEM_WIDTH,MENU_ITEM_HEIGHT);
        lv_obj_center(menu_item[i]);
        lv_obj_set_pos(menu_item[i],0,i*MENU_ITEM_MARGIN_Y);
        lv_obj_set_style_radius(menu_item[i],20,0);
        lv_obj_t* item_label=lv_label_create(menu_item[i]);
        lv_label_set_text(item_label,menu_item_text[i]);
        lv_obj_center(item_label);
    }

    top_layer=lv_layer_sys();         //顶层
    lv_obj_set_style_bg_opa(top_layer,60,0);
    lv_obj_t * btn_up =lv_btn_create(top_layer);
    lv_obj_t * btn_down =lv_btn_create(top_layer);
    lv_obj_t * btn_click=lv_btn_create(top_layer);
    lv_obj_t * up_label = lv_label_create(btn_up);
    lv_obj_t * down_label = lv_label_create(btn_down);
    lv_obj_t * click_label = lv_label_create(btn_click);
    lv_label_set_text(up_label, "up");
    lv_label_set_text(down_label, "down");
    lv_label_set_text(click_label, "click");

    lv_obj_center(up_label);
    lv_obj_center(down_label);
    lv_obj_center(click_label);
    lv_obj_align(btn_up, LV_ALIGN_CENTER, 150, -50);
    lv_obj_align(btn_down, LV_ALIGN_CENTER, 150, 50);
    lv_obj_align(btn_click, LV_ALIGN_CENTER, 150, 0);
    lv_obj_set_size(btn_up, 60 ,30);
    lv_obj_set_size(btn_down, 60 ,30);
    lv_obj_set_size(btn_click, 60 ,30);

    lv_obj_add_event_cb(btn_up, btn_up_event, LV_EVENT_CLICKED, container );
    lv_obj_add_event_cb(btn_down, btn_down_event, LV_EVENT_CLICKED, container );
    lv_obj_add_event_cb(btn_click, btn_click_event, LV_EVENT_CLICKED, container );

    lv_obj_scroll_to_view(menu_item[0], LV_ANIM_OFF);
}

void lv_img_menu_init(void) {
    scr = lv_scr_act();
    lv_obj_set_size(scr,lv_pct(100),lv_pct(100));
    lv_obj_set_style_bg_color(scr, lv_palette_lighten(LV_PALETTE_PURPLE,5), 0);

    // LV_IMG_DECLARE(camera);

    // lv_img_dsc_t app_img_list[] = {camera, camera, camera, camera, camera, camera};

    lv_obj_t * btn_up = lv_btn_create(scr);
    lv_obj_t * btn_down = lv_btn_create(scr);
    lv_obj_t * up_label = lv_label_create(btn_up);
    lv_obj_t * down_label = lv_label_create(btn_down);
    lv_label_set_text(up_label, "up");
    lv_label_set_text(down_label, "down");

    container = lv_obj_create(scr);
    lv_obj_set_size(container,MENU_CONTAINER_WIDHT,MENU_CONTAINER_HEIGHT);
    lv_obj_set_align(container,LV_ALIGN_CENTER);
    lv_obj_set_style_bg_color(container,lv_palette_lighten(LV_PALETTE_BLUE,3),0);
    lv_obj_set_style_border_width(container,0,0);

    for (int i=0;i<MENU_ITEM_COUNT;i++) {
        menu_item[i]=lv_imgbtn_create(container);
        lv_obj_set_size(menu_item[i],MENU_ITEM_WIDTH,MENU_ITEM_HEIGHT);
        lv_obj_center(menu_item[i]);
        lv_obj_set_pos(menu_item[i],0,i*MENU_ITEM_MARGIN_Y);
        lv_obj_set_style_radius(menu_item[i],20,0);
    }

    lv_obj_center(up_label);
    lv_obj_center(down_label);

    lv_obj_align(btn_up, LV_ALIGN_CENTER, 150, -50);
    lv_obj_align(btn_down, LV_ALIGN_CENTER, 150, 50);

    lv_obj_set_size(btn_up, 80 ,40);
    lv_obj_set_size(btn_down, 80 ,40);

    lv_obj_add_event_cb(btn_up, btn_up_event, LV_EVENT_CLICKED, container );
    lv_obj_add_event_cb(btn_down, btn_down_event, LV_EVENT_CLICKED, container );

    /*menu参数*/
    lv_obj_set_scroll_snap_y(container , LV_SCROLL_SNAP_CENTER );//设置控件捕捉
    lv_obj_set_scrollbar_mode(container , LV_SCROLLBAR_MODE_OFF);//隐藏滚动条

    //循环将app放入布局
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        lv_obj_t * app = lv_obj_get_child(container , i);
//        lv_img_dsc_t* img = &app_img_list[i];
//        lv_imgbtn_set_src(app, LV_IMGBTN_STATE_RELEASED, img,NULL,NULL);//给app控件绑定图片
//        lv_obj_set_size(app, (*img).header.w, (*img).header.h);//设置app控件的大小为图片大小（一定要将imgbtn大小设置成图片大小，否则无法对齐）
    }
    lv_obj_scroll_to_view(menu_item[0], LV_PART_MAIN);

    while(1) {
        lv_timer_handler();
        usleep(5*10);
    }
}

static int btn_sum=0;
static int prev_btn_sum=0;

void timer_page_cb(lv_timer_t* timer) {
    if (sum!=prev_sum) {
        if (sum>prev_sum) {
            btn_sum++;
        }else if (sum<prev_sum) {
            btn_sum--;
        }
        if (btn_sum>2) {
            btn_sum=1;
        }else if (btn_sum<1) {
            btn_sum=2;
        }
        prev_sum = sum;
    }
    if (btn_sum!=prev_btn_sum) {
        if (btn_sum==1) {
            lv_obj_clear_state(backBtn,LV_STATE_CHECKED);
            lv_obj_add_state(enterBtn,LV_STATE_CHECKED);
        }else if (btn_sum==2) {
            lv_obj_clear_state(enterBtn,LV_STATE_CHECKED);
            lv_obj_add_state(backBtn,LV_STATE_CHECKED);
        }
        prev_btn_sum = btn_sum;
    }
}
static lv_obj_t* title;

void page1() {
    container=lv_obj_create(scr);
    lv_obj_set_size(container,240,240);
    lv_obj_center(container);
    lv_obj_set_style_bg_color(container,lv_palette_lighten(LV_PALETTE_LIGHT_BLUE,3),0);
    lv_obj_set_style_border_width(container,0,0);

    title=lv_label_create(container);
    lv_label_set_text(title,"ITEM 1");
    lv_obj_center(title);

    enterBtn=lv_btn_create(container);
    lv_obj_set_size(enterBtn,60,30);
    lv_obj_align(enterBtn,LV_ALIGN_BOTTOM_MID,-50,-30);

    backBtn=lv_btn_create(container);
    lv_obj_set_size(backBtn,60,30);
    lv_obj_align(backBtn,LV_ALIGN_BOTTOM_MID,50,-30);

    lv_obj_t* lbl1=lv_label_create(enterBtn);
    lv_obj_center(lbl1);
    lv_label_set_text(lbl1,"Enter");
    lv_obj_t* lbl2=lv_label_create(backBtn);
    lv_obj_center(lbl2);
    lv_label_set_text(lbl2,"Back");
}
void page2() {

}
void page3() {

}
void page4() {

}
void page5() {

}
#define SETTING_ITEM_COUNT 5
void page6() {
    lv_obj_t* setting_item[SETTING_ITEM_COUNT];
    const char* item_text[SETTING_ITEM_COUNT]={"Setting1","Setting2","Setting3","Setting4","Setting5"};
    lv_obj_t* container=lv_obj_create(scr);
    lv_obj_set_size(container,240,240);
    lv_obj_center(container);
    lv_obj_set_style_bg_color(container,lv_palette_lighten(LV_PALETTE_LIGHT_BLUE,3),0);
    lv_obj_set_style_border_width(container,0,0);
    for (int i=0;i<SETTING_ITEM_COUNT;i++) {
        setting_item[i]=lv_btn_create(container);
        lv_obj_set_size(setting_item[i],180,40);
        lv_obj_set_align(setting_item[i],LV_ALIGN_TOP_MID);
        lv_obj_set_pos(setting_item[i],0,i*40);
        lv_obj_set_style_bg_color(setting_item[i],lv_color_white(),0);
        lv_obj_set_style_radius(setting_item[i],0,0);

        lv_obj_set_style_shadow_color(setting_item[i],lv_color_black(),0);
        lv_obj_set_style_shadow_width(setting_item[i],10,0);
        lv_obj_set_style_shadow_ofs_x(setting_item[i],2,0);

        lv_obj_t* item_txt=lv_label_create(setting_item[i]);
        lv_label_set_text(item_txt,item_text[i]);
        lv_obj_set_style_text_color(item_txt,lv_color_black(),0);
        lv_obj_center(item_txt);
    }
}

