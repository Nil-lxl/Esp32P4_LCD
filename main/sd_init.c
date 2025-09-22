#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "esp_vfs_fat.h"
#include "esp_check.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

#include "sd_card.h"
#include "dirent.h"

static const char *TAG = "SD_INIT";

#define MOUNT_PATH "/sdcard"

esp_err_t sdcard_init(void) {

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
    };

    sdmmc_card_t *card;
    const char *mount_path = MOUNT_PATH;

    ESP_LOGI(TAG, "Initializing SD card");

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

    sdmmc_slot_config_t slot_config = {
        .width = 4,
        .clk = SDMMC_PIN_CLK,
        .cmd = SDMMC_PIN_CMD,
        .d0 = SDMMC_PIN_D0,
        .d1 = SDMMC_PIN_D1,
        .d2 = SDMMC_PIN_D2,
        .d3 = SDMMC_PIN_D3,
        slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP,
    };

    ESP_LOGI(TAG, "Mouting Filesystem");

    ESP_RETURN_ON_ERROR(esp_vfs_fat_sdmmc_mount(mount_path, &host, &slot_config, &mount_config, &card), TAG, "Failed to mount SD card");

    // ESP_LOGI(TAG, "open %s", MOUNT_PATH);

    // DIR *card_dir = opendir(MOUNT_PATH);

    // struct dirent *dir;
    // while ((dir = readdir(card_dir)) != NULL) {
    //     printf("%s\n", dir->d_name);
    // }

    // closedir(card_dir);


    return ESP_OK;
}