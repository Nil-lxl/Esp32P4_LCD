#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_lcd_st7703.h"
#include "esp_lcd_st7703_720x720.h"
#include "esp_lcd_jd9365.h"
#include "esp_lcd_ek79007.h"
#include "lcd_h070b13.h"

#if CONFIG_LCD_USE_PANEL_EK79007
#define LCD_PIXEL_CLK_MHZ           52
#define LCD_H_RES                   1024
#define LCD_V_RES                   600
#define LCD_HSYNC                   10
#define LCD_HBP                     160
#define LCD_HFP                     160
#define LCD_VSYNC                   1
#define LCD_VBP                     23
#define LCD_VFP                     12

#elif CONFIG_LCD_USE_PANEL_H070B13
#define LCD_PIXEL_CLK_MHZ           60
#define LCD_H_RES                   800
#define LCD_V_RES                   1280
#define LCD_HSYNC                   20
#define LCD_HBP                     20
#define LCD_HFP                     20
#define LCD_VSYNC                   2
#define LCD_VBP                     30
#define LCD_VFP                     20

#elif CONFIG_LCD_USE_PANEL_ST7703
#define LCD_PIXEL_CLK_MHZ           40
#define LCD_H_RES                   640
#define LCD_V_RES                   480
#define LCD_HSYNC                   120
#define LCD_HBP                     120
#define LCD_HFP                     120
#define LCD_VSYNC                   5
#define LCD_VBP                     13
#define LCD_VFP                     17

#elif CONFIG_LCD_USE_PANEL_ST7703_720x720
#define LCD_PIXEL_CLK_MHZ           30
#define LCD_H_RES                   720
#define LCD_V_RES                   720
#define LCD_HSYNC                   20
#define LCD_HBP                     80
#define LCD_HFP                     80
#define LCD_VSYNC                   4
#define LCD_VBP                     12
#define LCD_VFP                     30

#elif CONFIG_LCD_USE_PANEL_JD9365
#define LCD_PIXEL_CLK_MHZ           35
#define LCD_H_RES                   800
#define LCD_V_RES                   1280
#define LCD_HSYNC                   24
#define LCD_HBP                     24
#define LCD_HFP                     30
#define LCD_VSYNC                   4
#define LCD_VBP                     12
#define LCD_VFP                     20
//RGB PANEL
#elif CONFIG_LCD_USE_PANEL_H035A17
#define LCD_PIXEL_CLK_HZ            (16 * 1000 * 1000)
#define LCD_H_RES                   640
#define LCD_V_RES                   480
#define LCD_HSYNC                   2
#define LCD_HBP                     20
#define LCD_HFP                     20
#define LCD_VSYNC                   2
#define LCD_VBP                     6
#define LCD_VFP                     20
#elif CONFIG_LCD_USE_DEFAULT_RGB_PANEL
#define LCD_PIXEL_CLK_HZ            (24 * 1000 * 1000)
#define LCD_H_RES                   1024
#define LCD_V_RES                   600
#define LCD_HSYNC                   24
#define LCD_HBP                     136
#define LCD_HFP                     160
#define LCD_VSYNC                   2
#define LCD_VBP                     21
#define LCD_VFP                     12

#endif

#define MIPI_DSI_LANE_NUM           2    // 2 data lanes
#define MIPI_DSI_LANE_BITRATE_MBPS  1000 // 500MGbps

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your Board Design //////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// The "VDD_MIPI_DPHY" should be supplied with 2.5V, it can source from the internal LDO regulator or from external LDO chip
#define MIPI_PHY_LDO_CHANNEL        3  // LDO_VO3 is connected to VDD_MIPI_DPHY
#define MIPI_PHY_LDO_VOLTAGE        2500

#define LCD_BK_LIGHT_ON_LEVEL       1
#define LCD_BK_LIGHT_OFF_LEVEL      0

#define PIN_NUM_BK_LIGHT            32
#define PIN_NUM_LCD_RST             33

#define TOUCH_I2C_SDA               28
#define TOUCH_I2C_SCL               29

#define TOUCH_PIN_INT               21
#define TOUCH_PIN_RTN               22

#define TOUCH_PAD_WIDTH             LCD_H_RES
#define TOUCH_PAD_HEIGHT            LCD_V_RES


#define LCD_PIN_NUM_PCLK                17
#define LCD_PIN_NUM_HSYNC               18
#define LCD_PIN_NUM_VSYNC               19
#define LCD_PIN_NUM_DE                  20

#define SPI_PIN_NUM_SDA                 23
#define SPI_PIN_NUM_SCL                 22
#define SPI_PIN_NUM_CS                  21

#define RGB_LCD_PIN_NUM_DATA0           9
#define RGB_LCD_PIN_NUM_DATA1           10
#define RGB_LCD_PIN_NUM_DATA2           11
#define RGB_LCD_PIN_NUM_DATA3           12
#define RGB_LCD_PIN_NUM_DATA4           13
#define RGB_LCD_PIN_NUM_DATA5           14
#define RGB_LCD_PIN_NUM_DATA6           15
#define RGB_LCD_PIN_NUM_DATA7           16

#define RGB_LCD_PIN_NUM_DATA8           54
#define RGB_LCD_PIN_NUM_DATA9           2
#define RGB_LCD_PIN_NUM_DATA10          3
#define RGB_LCD_PIN_NUM_DATA11          4
#define RGB_LCD_PIN_NUM_DATA12          5
#define RGB_LCD_PIN_NUM_DATA13          6
#define RGB_LCD_PIN_NUM_DATA14          7
#define RGB_LCD_PIN_NUM_DATA15          8

#define RGB_LCD_PIN_NUM_DATA16          46
#define RGB_LCD_PIN_NUM_DATA17          47
#define RGB_LCD_PIN_NUM_DATA18          48
#define RGB_LCD_PIN_NUM_DATA19          49
#define RGB_LCD_PIN_NUM_DATA20          50
#define RGB_LCD_PIN_NUM_DATA21          51
#define RGB_LCD_PIN_NUM_DATA22          52
#define RGB_LCD_PIN_NUM_DATA23          53

#if 1   //RGB 24Bit
#define LCD_DATA_BUS_WIDTH              24
#define LCD_BITS_PER_PIXEL              24
#define LCD_PIXEL_SIZE                  3
#define LV_COLOR_FORMAT            LV_COLOR_FORMAT_RGB888
#else   //RGB 16Bit
#define LCD_DATA_BUS_WIDTH              16
#define LCD_BITS_PER_PIXEL              16
#define LCD_PIXEL_SIZE                  2
#define LV_COLOR_FORMAT            LV_COLOR_FORMAT_RGB565

#endif 

#define LCD_BUFFER_COUNT                3
#define LCD_BUFFER_SIZE                 LCD_H_RES * LCD_V_RES * LCD_PIXEL_SIZE

#define LCD_PIN_NUM_DISP_EN             -1

#ifdef __cplusplus
}
#endif  //extern "C"

