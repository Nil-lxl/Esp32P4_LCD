#pragma once

#define SDMMC_PIN_CLK GPIO_NUM_43
#define SDMMC_PIN_CMD GPIO_NUM_44
#define SDMMC_PIN_D0 GPIO_NUM_39
#define SDMMC_PIN_D1 GPIO_NUM_40
#define SDMMC_PIN_D2 GPIO_NUM_41
#define SDMMC_PIN_D3 GPIO_NUM_42

esp_err_t sdcard_init(void);
