#include "string.h"
#include "lvgl.h"
#include "esp_log.h"
#include "esp_serial_slave_link/essl_sdio.h"
#include "at_sdio_cmd.h"

const char *TAG = "AT_UI";

static essl_handle_t at_handle;
static lv_obj_t *scr;
static lv_obj_t *output_area;

void btn_scan_cb(lv_event_t *e) {
    at_handle = get_sdio_slave();
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        const char *scan = "AT+CWJAP=\"MI14\",\"456123123\"\r\n";
        essl_send_packet(at_handle, scan, strlen(scan), portMAX_DELAY);

        uint8_t *data = get_at_result();
        ESP_LOGI("lv_get", "get data:%s", data);
    }

}

void at_send_cb(lv_event_t *e) {
    at_handle = get_sdio_slave();
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *area = (lv_obj_t *)lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        lv_textarea_set_text(output_area, "");

        const char *text = lv_textarea_get_text(area);
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "%s\r\n", text);
        essl_send_packet(at_handle, cmd, strlen(cmd), portMAX_DELAY);

        uint8_t *result = get_at_result();
        ESP_LOGI(TAG, "get data:%s", result);

        lv_textarea_set_text(output_area, (char *)result);

    }
}
void wifi_ui(void) {
    scr = lv_scr_act();

#if 0
    lv_obj_t *btn_scan_wifi = lv_btn_create(scr);
    lv_obj_center(btn_scan_wifi);
    lv_obj_set_size(btn_scan_wifi, 100, 100);
    lv_obj_add_event_cb(btn_scan_wifi, btn_scan_cb, LV_EVENT_CLICKED, NULL);
#else 
    lv_obj_t *input_area = lv_textarea_create(scr);
    lv_obj_set_size(input_area, 300, 60);
    lv_obj_set_align(input_area, LV_ALIGN_RIGHT_MID);
    lv_obj_set_pos(input_area, -100, -200);

    output_area = lv_textarea_create(scr);
    lv_obj_set_size(output_area, 600, 300);
    lv_obj_set_align(output_area, LV_ALIGN_TOP_LEFT);

    lv_obj_t *keyboard = lv_keyboard_create(scr);
    lv_obj_set_size(keyboard, 1024, 300);
    lv_obj_set_align(keyboard, LV_ALIGN_BOTTOM_MID);

    lv_keyboard_set_textarea(keyboard, input_area);

    lv_obj_t *send_btn = lv_btn_create(scr);
    lv_obj_set_size(send_btn, 100, 60);
    lv_obj_set_align(send_btn, LV_ALIGN_RIGHT_MID);
    lv_obj_set_pos(send_btn, -200, -100);

    lv_obj_t *send_label = lv_label_create(send_btn);
    lv_label_set_text(send_label, "send");
    lv_obj_set_align(send_label, LV_ALIGN_CENTER);
    lv_obj_add_event_cb(send_btn, at_send_cb, LV_EVENT_CLICKED, input_area);
#endif
}