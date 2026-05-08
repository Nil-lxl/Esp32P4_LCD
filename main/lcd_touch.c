#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_timer.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"

#include "driver/i2c_master.h"
#include "driver/gpio.h"

#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_lcd_touch_gt1151.h"


#include "lcd_config.h"

static const char *TAG = "LCD Touch";

static i2c_master_bus_handle_t i2c_bus_handle;
static esp_lcd_touch_handle_t touch_handle;

esp_err_t lcd_touch_init(esp_lcd_touch_handle_t *touch) {
    i2c_master_bus_config_t i2c_bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .sda_io_num = TOUCH_I2C_SDA,
        .scl_io_num = TOUCH_I2C_SCL,
        .i2c_port = I2C_NUM_0,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_handle));
    const esp_lcd_touch_config_t touch_config = {
        .x_max = TOUCH_PAD_WIDTH,
        .y_max = TOUCH_PAD_HEIGHT,
        .rst_gpio_num = GPIO_NUM_NC,
        .int_gpio_num = GPIO_NUM_NC,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    esp_lcd_panel_io_handle_t touch_io_handle;
    esp_lcd_panel_io_i2c_config_t touch_io_cfg = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    touch_io_cfg.scl_speed_hz = 400000;

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c_v2(i2c_bus_handle, &touch_io_cfg, &touch_io_handle));
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(touch_io_handle, &touch_config, &touch_handle));
    // ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt1151(touch_io_handle, &touch_config, &touch_handle));


    if (touch) {
        *touch = touch_handle;
    }
    return ESP_OK;
}
