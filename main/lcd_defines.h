#ifndef _LCD_DEFINES_H
#define _LCD_DEFINES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_lcd_st7703.h"
#include "esp_lcd_st7703_720x720.h"
#include "esp_lcd_jd9365.h"
#include "esp_lcd_ek79007.h"
#include "vernon_gt911.h"

static const char *TAG = "example";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD Spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if CONFIG_EXAMPLE_LCD_USE_EK79007
#define EXAMPLE_MIPI_DSI_DPI_CLK_MHZ  40
#define EXAMPLE_MIPI_DSI_LCD_H_RES    1024
#define EXAMPLE_MIPI_DSI_LCD_V_RES    600
#define EXAMPLE_MIPI_DSI_LCD_HSYNC    10
#define EXAMPLE_MIPI_DSI_LCD_HBP      120
#define EXAMPLE_MIPI_DSI_LCD_HFP      120
#define EXAMPLE_MIPI_DSI_LCD_VSYNC    1
#define EXAMPLE_MIPI_DSI_LCD_VBP      20
#define EXAMPLE_MIPI_DSI_LCD_VFP      10
#elif CONFIG_EXAMPLE_LCD_USE_ST7703
#define EXAMPLE_MIPI_DSI_DPI_CLK_MHZ  40
#define EXAMPLE_MIPI_DSI_LCD_H_RES    640
#define EXAMPLE_MIPI_DSI_LCD_V_RES    480
#define EXAMPLE_MIPI_DSI_LCD_HSYNC    120
#define EXAMPLE_MIPI_DSI_LCD_HBP      120
#define EXAMPLE_MIPI_DSI_LCD_HFP      120
#define EXAMPLE_MIPI_DSI_LCD_VSYNC    5
#define EXAMPLE_MIPI_DSI_LCD_VBP      13
#define EXAMPLE_MIPI_DSI_LCD_VFP      17

#elif CONFIG_EXAMPLE_LCD_USE_ST7703_720x720
#define EXAMPLE_MIPI_DSI_DPI_CLK_MHZ  30
#define EXAMPLE_MIPI_DSI_LCD_H_RES    720
#define EXAMPLE_MIPI_DSI_LCD_V_RES    720
#define EXAMPLE_MIPI_DSI_LCD_HSYNC    20
#define EXAMPLE_MIPI_DSI_LCD_HBP      80
#define EXAMPLE_MIPI_DSI_LCD_HFP      80
#define EXAMPLE_MIPI_DSI_LCD_VSYNC    4
#define EXAMPLE_MIPI_DSI_LCD_VBP      12
#define EXAMPLE_MIPI_DSI_LCD_VFP      30

#elif CONFIG_EXAMPLE_LCD_USE_JD9365
#define EXAMPLE_MIPI_DSI_DPI_CLK_MHZ  35
#define EXAMPLE_MIPI_DSI_LCD_H_RES    800
#define EXAMPLE_MIPI_DSI_LCD_V_RES    1280
#define EXAMPLE_MIPI_DSI_LCD_HSYNC    24
#define EXAMPLE_MIPI_DSI_LCD_HBP      24
#define EXAMPLE_MIPI_DSI_LCD_HFP      30
#define EXAMPLE_MIPI_DSI_LCD_VSYNC    4
#define EXAMPLE_MIPI_DSI_LCD_VBP      12
#define EXAMPLE_MIPI_DSI_LCD_VFP      20

#endif

#define EXAMPLE_MIPI_DSI_LANE_NUM          2    // 2 data lanes
#define EXAMPLE_MIPI_DSI_LANE_BITRATE_MBPS 500 // 500MGbps

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your Board Design //////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// The "VDD_MIPI_DPHY" should be supplied with 2.5V, it can source from the internal LDO regulator or from external LDO chip
#define EXAMPLE_MIPI_DSI_PHY_PWR_LDO_CHAN       3  // LDO_VO3 is connected to VDD_MIPI_DPHY
#define EXAMPLE_MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV 2500
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL           1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL          !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_BK_LIGHT                -1

#if CONFIG_EXAMPLE_LCD_USE_EK79007
//LCD RST PIN
#define EXAMPLE_PIN_NUM_LCD_RST                 4
#else
#define EXAMPLE_PIN_NUM_LCD_RST                 24
#endif

#if CONFIG_EXAMPLE_MONITOR_REFRESH_BY_GPIO
#define EXAMPLE_PIN_NUM_REFRESH_MONITOR         20  // Monitor the Refresh Rate by toggling the GPIO
#endif

#ifdef CONFIG_EXAMPLE_LCD_USE_TOUCH_ENABLED
#define TOUCH_GT911_SDA  7
#define TOUCH_GT911_SCL  8
#define TOUCH_GT911_INT  21
#define TOUCH_GT911_RTN  22

#define TOUCH_PAD_WIDTH  1024
#define TOUCH_PAD_HEIGHT 600
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your Application ///////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define EXAMPLE_LVGL_DRAW_BUF_LINES    (EXAMPLE_MIPI_DSI_LCD_V_RES / 10) // number of display lines in each draw buffer
#define EXAMPLE_LVGL_TICK_PERIOD_MS    2
#define EXAMPLE_LVGL_TASK_STACK_SIZE   (48 * 1024)
#define EXAMPLE_LVGL_TASK_PRIORITY     2

#ifdef __cplusplus
}
#endif  //extern "C"

#endif //_LCD_DEFINES_H
