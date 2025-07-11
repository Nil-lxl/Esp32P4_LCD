/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "string.h"
#include "lvgl.h"
#include "esp_log.h"
#include "demos/lv_demos.h"
#include "src/ui/ui.h"

#include "esp_serial_slave_link/essl_sdio.h"
#include "at_sdio_cmd.h"

static essl_handle_t at_handle;
extern SemaphoreHandle_t at_result_mutex;
extern QueueHandle_t at_result_queue;

void wifi_ui(void);

void example_lvgl_demo_ui(lv_display_t *disp) {
    // lv_demo_benchmark();
    // lv_demo_widgets();
    // UI_init();
    wifi_ui();
}


void get_result() {
    uint8_t result[2048];
    // if (xQueueReceiveFromISR(at_result_queue, (void *)result, NULL) == pdTRUE) {
    //     ESP_LOGI(NULL, "get result success");
    //     ESP_LOGI(NULL, "result:%s", result);
    // }
    // xSemaphoreTake(at_result_mutex, portMAX_DELAY);

    // if (xSemaphoreTake(at_result_mutex, portMAX_DELAY)) {
    //     ESP_LOGI(NULL, "get result success");
    // } else {
    //     ESP_LOGE(NULL, "get result failed");
    // }

    // xSemaphoreGive(at_result_mutex);
}
void btn_scan_cb(lv_event_t *e) {
    int ret = 0;
    at_handle = get_sdio_slave();
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        const char *cmd = "AT+GMR\r\n";
        ret = essl_send_packet(at_handle, cmd, strlen(cmd), portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE("TAG", "Essl Send Failed,Err Code:%d", ret);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
        get_result();

    }

}
void wifi_ui(void) {

    lv_obj_t *scr = lv_scr_act();

    lv_obj_t *btn_scan_wifi = lv_btn_create(scr);
    lv_obj_center(btn_scan_wifi);
    lv_obj_set_size(btn_scan_wifi, 100, 50);

    lv_obj_add_event_cb(btn_scan_wifi, btn_scan_cb, LV_EVENT_CLICKED, NULL);
}

