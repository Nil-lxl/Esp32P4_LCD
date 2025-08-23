#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_lcd_st7703.h"
#include "esp_lcd_st7703_720x720.h"
#include "esp_lcd_jd9365.h"
#include "esp_lcd_ek79007.h"
#include "lcd_h070b13.h"

static const char *TAG = "MIPI_LCD";

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////// Please update the following configuration according to your LCD Spec //////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if CONFIG_EXAMPLE_LCD_USE_EK79007
#define MIPI_DSI_PIXEL_CLK_MHZ  40
#define MIPI_DSI_LCD_H_RES    1024
#define MIPI_DSI_LCD_V_RES    600
#define MIPI_DSI_LCD_HSYNC    10
#define MIPI_DSI_LCD_HBP      120
#define MIPI_DSI_LCD_HFP      120
#define MIPI_DSI_LCD_VSYNC    1
#define MIPI_DSI_LCD_VBP      20
#define MIPI_DSI_LCD_VFP      10

#elif CONFIG_EXAMPLE_LCD_USE_H070B13
#define MIPI_DSI_PIXEL_CLK_MHZ  58
#define MIPI_DSI_LCD_H_RES    800
#define MIPI_DSI_LCD_V_RES    1280
#define MIPI_DSI_LCD_HSYNC    20
#define MIPI_DSI_LCD_HBP      20
#define MIPI_DSI_LCD_HFP      20
#define MIPI_DSI_LCD_VSYNC    2
#define MIPI_DSI_LCD_VBP      30
#define MIPI_DSI_LCD_VFP      20

#elif CONFIG_EXAMPLE_LCD_USE_ST7703
#define MIPI_DSI_PIXEL_CLK_MHZ  40
#define MIPI_DSI_LCD_H_RES    640
#define MIPI_DSI_LCD_V_RES    480
#define MIPI_DSI_LCD_HSYNC    120
#define MIPI_DSI_LCD_HBP      120
#define MIPI_DSI_LCD_HFP      120
#define MIPI_DSI_LCD_VSYNC    5
#define MIPI_DSI_LCD_VBP      13
#define MIPI_DSI_LCD_VFP      17

#elif CONFIG_EXAMPLE_LCD_USE_ST7703_720x720
#define MIPI_DSI_PIXEL_CLK_MHZ  30
#define MIPI_DSI_LCD_H_RES    720
#define MIPI_DSI_LCD_V_RES    720
#define MIPI_DSI_LCD_HSYNC    20
#define MIPI_DSI_LCD_HBP      80
#define MIPI_DSI_LCD_HFP      80
#define MIPI_DSI_LCD_VSYNC    4
#define MIPI_DSI_LCD_VBP      12
#define MIPI_DSI_LCD_VFP      30

#elif CONFIG_EXAMPLE_LCD_USE_JD9365
#define MIPI_DSI_PIXEL_CLK_MHZ  35
#define MIPI_DSI_LCD_H_RES    800
#define MIPI_DSI_LCD_V_RES    1280
#define MIPI_DSI_LCD_HSYNC    24
#define MIPI_DSI_LCD_HBP      24
#define MIPI_DSI_LCD_HFP      30
#define MIPI_DSI_LCD_VSYNC    4
#define MIPI_DSI_LCD_VBP      12
#define MIPI_DSI_LCD_VFP      20

#endif

#define MIPI_DSI_LANE_NUM          2    // 2 data lanes
#define MIPI_DSI_LANE_BITRATE_MBPS 1000 // 500MGbps

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your Board Design //////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// The "VDD_MIPI_DPHY" should be supplied with 2.5V, it can source from the internal LDO regulator or from external LDO chip
#define MIPI_DSI_PHY_PWR_LDO_CHAN       3  // LDO_VO3 is connected to VDD_MIPI_DPHY
#define MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV 2500

#define LCD_BK_LIGHT_ON_LEVEL           1
#define LCD_BK_LIGHT_OFF_LEVEL          !LCD_BK_LIGHT_ON_LEVEL
#define PIN_NUM_BK_LIGHT                32

#if CONFIG_EXAMPLE_LCD_USE_EK79007
//LCD RST PIN
#define EXAMPLE_PIN_NUM_LCD_RST                 4
#else
#define EXAMPLE_PIN_NUM_LCD_RST                 33
#endif

#define TOUCH_I2C_SDA       7
#define TOUCH_I2C_SCL       8
#define TOUCH_PIN_INT       21
#define TOUCH_PIN_RTN       22

#define TOUCH_PAD_WIDTH     MIPI_DSI_LCD_H_RES
#define TOUCH_PAD_HEIGHT    MIPI_DSI_LCD_V_RES

#ifdef __cplusplus
}
#endif  //extern "C"

