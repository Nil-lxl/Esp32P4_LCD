#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_types.h"
#include "esp_lcd_panel_io_additions.h"
#include "esp_io_expander.h"

#include "esp_cache.h"
#include "esp_dma_utils.h"
#include "esp_private/esp_cache_private.h"

#include "esp_log.h"
#include "esp_check.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"

#include "driver/gpio.h"
#include "driver/jpeg_decode.h"

#include "lcd_config.h"
// #include "lcd_h035a17.h"

#if CONFIG_LCD_USE_RGB_INTERFACE

static const char *TAG = "RGB_DRIVER";

static esp_lcd_panel_handle_t rgb_panel_handle;
static esp_lcd_panel_io_handle_t rgb_io_handle;

esp_err_t rgb_lcd_init(esp_lcd_panel_handle_t *panel_handle) {
    ESP_LOGI(TAG, "Install RGB LCD panel driver");
    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = LCD_DATA_BUS_WIDTH,
        .bits_per_pixel= LCD_BITS_PER_PIXEL,
        // .bounce_buffer_size_px = 40 * LCD_H_RES,
        .dma_burst_size = 64,
        .num_fbs = LCD_BUFFER_COUNT,
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .disp_gpio_num = LCD_PIN_NUM_DISP_EN,
        .pclk_gpio_num = LCD_PIN_NUM_PCLK,
        .vsync_gpio_num = LCD_PIN_NUM_VSYNC,
        .hsync_gpio_num = LCD_PIN_NUM_HSYNC,
        .de_gpio_num = LCD_PIN_NUM_DE,
        .data_gpio_nums = {
            RGB_LCD_PIN_NUM_DATA0,
            RGB_LCD_PIN_NUM_DATA1,
            RGB_LCD_PIN_NUM_DATA2,
            RGB_LCD_PIN_NUM_DATA3,
            RGB_LCD_PIN_NUM_DATA4,
            RGB_LCD_PIN_NUM_DATA5,
            RGB_LCD_PIN_NUM_DATA6,
            RGB_LCD_PIN_NUM_DATA7,
            RGB_LCD_PIN_NUM_DATA8,
            RGB_LCD_PIN_NUM_DATA9,
            RGB_LCD_PIN_NUM_DATA10,
            RGB_LCD_PIN_NUM_DATA11,
            RGB_LCD_PIN_NUM_DATA12,
            RGB_LCD_PIN_NUM_DATA13,
            RGB_LCD_PIN_NUM_DATA14,
            RGB_LCD_PIN_NUM_DATA15,
            RGB_LCD_PIN_NUM_DATA16,
            RGB_LCD_PIN_NUM_DATA17,
            RGB_LCD_PIN_NUM_DATA18,
            RGB_LCD_PIN_NUM_DATA19,
            RGB_LCD_PIN_NUM_DATA20,
            RGB_LCD_PIN_NUM_DATA21,
            RGB_LCD_PIN_NUM_DATA22,
            RGB_LCD_PIN_NUM_DATA23,
        },
        .timings = {
            .pclk_hz = LCD_PIXEL_CLK_HZ,
            .h_res = LCD_H_RES,
            .v_res = LCD_V_RES,
            .hsync_back_porch = LCD_HBP,
            .hsync_front_porch = LCD_HFP,
            .hsync_pulse_width = LCD_HSYNC,
            .vsync_back_porch = LCD_VBP,
            .vsync_front_porch = LCD_VFP,
            .vsync_pulse_width = LCD_VSYNC,
            .flags = {
                .pclk_active_neg = true
            },
        },
        .flags.fb_in_psram = true, // allocate frame buffer in PSRAM
    };


#ifdef CONFIG_RGB_LCD_USE_SPI_INIT

    ESP_LOGI(TAG, "Initialize 3-Wire SPI Panel IO");
    spi_line_config_t line_config = {
        .cs_io_type = IO_TYPE_GPIO,
        .cs_gpio_num = SPI_PIN_NUM_CS,
        .scl_io_type = IO_TYPE_GPIO,
        .scl_gpio_num = SPI_PIN_NUM_SCL,
        .sda_io_type = IO_TYPE_GPIO,
        .sda_gpio_num = SPI_PIN_NUM_SDA,
        .io_expander = NULL,
    };
#ifdef CONFIG_LCD_USE_PANEL_H035A17  
    h035a17_vendor_config_t vendor_config = {
#endif
        .rgb_config = &panel_config,
        .flags = {
            .mirror_by_cmd = 1,
            .enable_io_multiplex = 0,
        }
    };

    esp_lcd_panel_dev_config_t panel_dev_config = {
        .reset_gpio_num = PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = LCD_BITS_PER_PIXEL,
        .vendor_config = &vendor_config,
    };

#ifdef CONFIG_LCD_USE_PANEL_H035A17
    esp_lcd_panel_io_3wire_spi_config_t io_config = H035A17_PANEL_IO_3WIRE_SPI_CONFIG(line_config, 0);
#endif

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_3wire_spi(&io_config, &rgb_io_handle));

#ifdef CONFIG_LCD_USE_PANEL_H035A17
    ESP_ERROR_CHECK(esp_lcd_new_panel_h035a17(rgb_io_handle, &panel_dev_config, &rgb_panel_handle));
#endif

#else
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &rgb_panel_handle));
#endif

    ESP_LOGI(TAG, "Initialize RGB LCD panel");
    ESP_ERROR_CHECK(esp_lcd_panel_reset(rgb_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(rgb_panel_handle));
    // ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(rgb_panel_handle, true));

    if (panel_handle) {
        *panel_handle = rgb_panel_handle;
    }

    return ESP_OK;
}

#endif

