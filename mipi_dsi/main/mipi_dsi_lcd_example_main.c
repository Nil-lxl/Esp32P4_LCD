/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_ldo_regulator.h"
#include "driver/gpio.h"
#include "soc/gpio_sig_map.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"

#include "lcd_defines.h"


// LVGL library is not thread-safe, this example will call LVGL APIs from different tasks, so use a mutex to protect it
static _lock_t lvgl_api_lock;

extern void example_lvgl_demo_ui(lv_display_t *disp);

static void example_lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_handle_t panel_handle = lv_display_get_user_data(disp);
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // pass the draw buffer to the driver
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, px_map);
}

Vernon_GT911 vernonGt911;
#ifdef CONFIG_EXAMPLE_LCD_USE_TOUCH_ENABLED

static void example_lvgl_touch_cb(lv_indev_t* indev,lv_indev_data_t* data){
    uint16_t x,y;
    if(GT911_touched(&vernonGt911)){
        GT911_read_pos(&vernonGt911,&x,&y,0);
        data->point.x=x;
        data->point.y=y;
        data->state=LV_INDEV_STATE_PRESSED;
    }else{
        data->state=LV_INDEV_STATE_RELEASED;
    }
        
}
#endif

static void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

static void example_lvgl_port_task(void *arg)
{
    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t time_till_next_ms = 0;
    while (1) {
        _lock_acquire(&lvgl_api_lock);
        time_till_next_ms = lv_timer_handler();
        _lock_release(&lvgl_api_lock);

        // in case of task watch dog timeout, set the minimal delay to 10ms
        if (time_till_next_ms < 10) {
            time_till_next_ms = 10;
        }
        usleep(1000 * time_till_next_ms);
    }
}

static bool example_notify_lvgl_flush_ready(esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t *edata, void *user_ctx)
{
    lv_display_t *disp = (lv_display_t *)user_ctx;
    lv_display_flush_ready(disp);
    return false;
}

#if CONFIG_EXAMPLE_MONITOR_REFRESH_BY_GPIO
static bool example_monitor_refresh_rate(esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t *edata, void *user_ctx)
{
    static int io_level = 0;
    // please note, the real refresh rate should be 2*frequency of this GPIO toggling
    gpio_set_level(EXAMPLE_PIN_NUM_REFRESH_MONITOR, io_level);
    io_level = !io_level;
    return false;
}
#endif

static void example_bsp_enable_dsi_phy_power(void)
{
    // Turn on the power for MIPI DSI PHY, so it can go from "No Power" state to "Shutdown" state
    esp_ldo_channel_handle_t ldo_mipi_phy = NULL;
#ifdef EXAMPLE_MIPI_DSI_PHY_PWR_LDO_CHAN
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id = EXAMPLE_MIPI_DSI_PHY_PWR_LDO_CHAN,
        .voltage_mv = EXAMPLE_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy));
    ESP_LOGI(TAG, "MIPI DSI PHY Powered on");
#endif
}

static void example_bsp_init_lcd_backlight(void)
{
#if EXAMPLE_PIN_NUM_BK_LIGHT >= 0
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
#endif
}

static void example_bsp_set_lcd_backlight(uint32_t level)
{
#if EXAMPLE_PIN_NUM_BK_LIGHT >= 0
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, level);
#endif
}

#if CONFIG_EXAMPLE_MONITOR_REFRESH_BY_GPIO
static void example_bsp_init_refresh_monitor_io(void)
{
    gpio_config_t monitor_io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_REFRESH_MONITOR,
    };
    ESP_ERROR_CHECK(gpio_config(&monitor_io_conf));
}
#endif


void GT911_test(void *param){
    uint16_t x,y;
    while (1){
        if(GT911_touched(&vernonGt911)){
            //
            // for(int i=0;i<5;i++){
                GT911_read_pos(&vernonGt911,&x,&y,0);
                ESP_LOGW(TAG,"No: %d, touched x: %d, touched y: %d\n", 0,  x, y);
            // }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

}

void app_main(void)
{
#ifdef CONFIG_EXAMPLE_LCD_USE_TOUCH_ENABLED
    GT911_init(&vernonGt911, TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT,
               TOUCH_GT911_RTN, I2C_NUM_0,GT911_ADDR1,
               TOUCH_PAD_WIDTH, TOUCH_PAD_HEIGHT);

    GT911_setRotation(&vernonGt911,ROTATION_NORMAL);
    ESP_LOGW(TAG,"GT911 TouchPad Init");
#endif    

#if CONFIG_EXAMPLE_MONITOR_REFRESH_BY_GPIO
    example_bsp_init_refresh_monitor_io();
#endif

    example_bsp_enable_dsi_phy_power();
    example_bsp_init_lcd_backlight();
    example_bsp_set_lcd_backlight(EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL);

    // create MIPI DSI bus first, it will initialize the DSI PHY as well
    esp_lcd_dsi_bus_handle_t mipi_dsi_bus;
    esp_lcd_dsi_bus_config_t bus_config = {
        .bus_id = 0,
        .num_data_lanes = EXAMPLE_MIPI_DSI_LANE_NUM,
        .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
        .lane_bit_rate_mbps = EXAMPLE_MIPI_DSI_LANE_BITRATE_MBPS,
    };
    ESP_ERROR_CHECK(esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus));

    ESP_LOGI(TAG, "Install MIPI DSI LCD control IO");
    esp_lcd_panel_io_handle_t mipi_dbi_io;
    // we use DBI interface to send LCD commands and parameters
    esp_lcd_dbi_io_config_t dbi_config = {
        .virtual_channel = 0,
        .lcd_cmd_bits = 8,   // according to the LCD spec
        .lcd_param_bits = 8, // according to the LCD spec
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &mipi_dbi_io));

    ESP_LOGI(TAG, "Install MIPI DSI LCD data panel");
    esp_lcd_panel_handle_t mipi_dpi_panel = NULL;
    esp_lcd_dpi_panel_config_t dpi_config = {
        .virtual_channel = 0,
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
        .dpi_clock_freq_mhz = EXAMPLE_MIPI_DSI_DPI_CLK_MHZ,
        .in_color_format = LCD_COLOR_FMT_RGB888,
        .video_timing = {
            .h_size = EXAMPLE_MIPI_DSI_LCD_H_RES,
            .v_size = EXAMPLE_MIPI_DSI_LCD_V_RES,
            .hsync_back_porch = EXAMPLE_MIPI_DSI_LCD_HBP,
            .hsync_pulse_width = EXAMPLE_MIPI_DSI_LCD_HSYNC,
            .hsync_front_porch = EXAMPLE_MIPI_DSI_LCD_HFP,
            .vsync_back_porch = EXAMPLE_MIPI_DSI_LCD_VBP,
            .vsync_pulse_width = EXAMPLE_MIPI_DSI_LCD_VSYNC,
            .vsync_front_porch = EXAMPLE_MIPI_DSI_LCD_VFP,
        },
#if CONFIG_EXAMPLE_USE_DMA2D_COPY_FRAME
        .flags.use_dma2d = true, // use DMA2D to copy draw buffer into frame buffer
#endif
    };

#if CONFIG_EXAMPLE_LCD_USE_EK79007
    ek79007_vendor_config_t vendor_config = {
        .mipi_config = {
            .dsi_bus = mipi_dsi_bus,
            .dpi_config = &dpi_config,
        },
    };
    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 24,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ek79007(mipi_dbi_io, &lcd_dev_config, &mipi_dpi_panel));
#elif CONFIG_EXAMPLE_LCD_USE_ST7703
    st7703_vendor_config_t vendor_config = {
        .mipi_config = {
            .dsi_bus = mipi_dsi_bus,
            .dpi_config = &dpi_config,
        },
    };
    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 24,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7703(mipi_dbi_io, &lcd_dev_config, &mipi_dpi_panel));

#elif CONFIG_EXAMPLE_LCD_USE_ST7703_720x720
    st7703_720_vendor_config_t vendor_config = {
        .mipi_config = {
            .dsi_bus = mipi_dsi_bus,
            .dpi_config = &dpi_config,
        },
    };
    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 24,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7703_720(mipi_dbi_io, &lcd_dev_config, &mipi_dpi_panel));

#elif CONFIG_EXAMPLE_LCD_USE_JD9365
    jd9365_vendor_config_t vendor_config = {
        .mipi_config = {
            .dsi_bus = mipi_dsi_bus,
            .dpi_config = &dpi_config,
        },
    };
    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 24,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_jd9365(mipi_dbi_io, &lcd_dev_config, &mipi_dpi_panel));
#endif

    ESP_ERROR_CHECK(esp_lcd_panel_reset(mipi_dpi_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(mipi_dpi_panel));
    // ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(mipi_dpi_panel,true));
    // turn on backlight
    example_bsp_set_lcd_backlight(EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    // create a lvgl display
    lv_display_t *display = lv_display_create(EXAMPLE_MIPI_DSI_LCD_H_RES, EXAMPLE_MIPI_DSI_LCD_V_RES);
    // associate the mipi panel handle to the display
    lv_display_set_user_data(display, mipi_dpi_panel);
    // set color depth
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB888);
    // create draw buffer
    void *buf1 = NULL;
    void *buf2 = NULL;
    ESP_LOGI(TAG, "Allocate separate LVGL draw buffers");
    // Note:
    // Keep the display buffer in **internal** RAM can speed up the UI because LVGL uses it a lot and it should have a fast access time
    // This example allocate the buffer from PSRAM mainly because we want to save the internal RAM
    
    size_t draw_buffer_sz = EXAMPLE_MIPI_DSI_LCD_H_RES * EXAMPLE_LVGL_DRAW_BUF_LINES * sizeof(lv_color_t);
    buf1 = heap_caps_malloc(draw_buffer_sz, MALLOC_CAP_SPIRAM);
    assert(buf1);
    buf2 = heap_caps_malloc(draw_buffer_sz, MALLOC_CAP_SPIRAM);
    assert(buf2);
    ESP_LOGI(TAG,"buffer1:%zd,buffer2:%zd,draw_buffer:%zd",sizeof(buf1),sizeof(buf2),draw_buffer_sz);

    // initialize LVGL draw buffers
    lv_display_set_buffers(display, buf1, buf2, draw_buffer_sz, LV_DISPLAY_RENDER_MODE_PARTIAL);
    // set the callback which can copy the rendered image to an area of the display
    lv_display_set_flush_cb(display, example_lvgl_flush_cb);

    lv_display_set_rotation(display,LV_DISPLAY_ROTATION_180);

    ESP_LOGI(TAG, "Register DPI panel event callback for LVGL flush ready notification");
    esp_lcd_dpi_panel_event_callbacks_t cbs = {
        .on_color_trans_done = example_notify_lvgl_flush_ready,
#if CONFIG_EXAMPLE_MONITOR_REFRESH_BY_GPIO
        .on_refresh_done = example_monitor_refresh_rate,
#endif
    };
    ESP_ERROR_CHECK(esp_lcd_dpi_panel_register_event_callbacks(mipi_dpi_panel, &cbs, display));

    ESP_LOGI(TAG, "Use esp_timer as LVGL tick timer");
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

#ifdef CONFIG_EXAMPLE_LCD_USE_TOUCH_ENABLED
    static lv_indev_t* touch_indev;
    touch_indev=lv_indev_create();
    lv_indev_set_type(touch_indev,LV_INDEV_TYPE_POINTER);
    lv_indev_set_display(touch_indev,display);
    lv_indev_set_read_cb(touch_indev,example_lvgl_touch_cb);
#endif     

    ESP_LOGI(TAG, "Create LVGL task");
    xTaskCreate(example_lvgl_port_task, "LVGL", EXAMPLE_LVGL_TASK_STACK_SIZE, NULL, EXAMPLE_LVGL_TASK_PRIORITY, NULL);

    ESP_LOGI(TAG, "Display LVGL Meter Widget");
    _lock_acquire(&lvgl_api_lock);
    example_lvgl_demo_ui(display);
    _lock_release(&lvgl_api_lock);
}