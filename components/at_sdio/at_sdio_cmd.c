#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "driver/uart.h"
#include "sdmmc_cmd.h"

#include "esp_log.h"
#include "esp_attr.h"

#include "esp_serial_slave_link/essl_sdio.h"
#include "at_sdio_cmd.h"

#define SDIO_CLK    GPIO_NUM_18
#define SDIO_CMD    GPIO_NUM_19
#define SDIO_D0     GPIO_NUM_14
#define SDIO_D1     GPIO_NUM_15
#define SDIO_D2     GPIO_NUM_16
#define SDIO_D3     GPIO_NUM_17

#define WIFI_POWER_ON GPIO_NUM_54


static const char *TAG = "SDIO_AT";

// static SemaphoreHandle_t slave_send_cmd_sem;
QueueHandle_t at_result_queue;
static SemaphoreHandle_t at_result_mutex;
static essl_handle_t slave_handle;

essl_handle_t get_sdio_slave(void) {
    return slave_handle;
}
esp_err_t sdio_power_on(void) {
    gpio_config_t slave_pwr = {
        .pin_bit_mask = (1ULL << (WIFI_POWER_ON)),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = false,
        .pull_down_en = false,
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&slave_pwr);

    gpio_set_level(WIFI_POWER_ON, 0);
    vTaskDelay(100);
    gpio_set_level(WIFI_POWER_ON, 1);
    vTaskDelay(100);

    return ESP_OK;
}

esp_err_t uart_init(uart_port_t num) {
    // uart_driver_install(uart_num, 2048, 2048, 20, &uart_queue, 0);
    return ESP_OK;
}

esp_err_t sdio_slave_init(essl_handle_t *handle) {
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.flags = SDMMC_HOST_FLAG_4BIT;
    host.input_delay_phase = SDMMC_DELAY_PHASE_2;
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
    host.flags |= SDMMC_HOST_FLAG_ALLOC_ALIGNED_BUF;

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 4;
    slot_config.clk = SDIO_CLK;
    slot_config.cmd = SDIO_CMD;
    slot_config.d0 = SDIO_D0;
    slot_config.d1 = SDIO_D1;
    slot_config.d2 = SDIO_D2;
    slot_config.d3 = SDIO_D3;

    ESP_ERROR_CHECK(sdmmc_host_init());
    ESP_ERROR_CHECK(sdmmc_host_init_slot(SDMMC_HOST_SLOT_1, &slot_config));

    sdmmc_card_t *slave = (sdmmc_card_t *)malloc(sizeof(sdmmc_card_t));
    if (slave == NULL) {
        return ESP_ERR_NO_MEM;
    }
    while (1) {
        if (sdmmc_card_init(&host, slave) == ESP_OK) {
            break;
        }
        ESP_LOGW(TAG, "Slave Init Failed, Retry...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    sdmmc_card_print_info(stdout, slave);

    essl_sdio_config_t slave_config = {
        .card = slave,
        .recv_buffer_size = 512,
    };

    ESP_ERROR_CHECK(essl_sdio_init_dev(handle, &slave_config));
    ESP_ERROR_CHECK(essl_init(*handle, portMAX_DELAY));

    return ESP_OK;
}

esp_err_t sdio_slave_reset(essl_handle_t handle) {
    esp_err_t ret;
    ESP_LOGI(TAG, "Reset Slave Device");
    ret = essl_write_reg(handle, 0, 1, NULL, portMAX_DELAY);
    if (ret != ESP_OK) {
        return ret;
    }
    ret = essl_send_slave_intr(handle, BIT(0), portMAX_DELAY);
    if (ret != ESP_OK) {
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(200));
    ret = essl_wait_for_ready(handle, portMAX_DELAY);
    ESP_LOGI(TAG, "slave io ready");

    return ret;
}

void sdio_receive_task(void *param) {
    // init_at_result(&at_result);
    size_t buf_size = 1024 * 2;
    uint8_t read_buf[buf_size];
    at_result_queue = xQueueCreate(20, 1024);
    if (at_result_queue == NULL) {
        ESP_LOGE(TAG, "create queue failed");
    }

    char *result = malloc(1024);
    bzero(result, 1024);

    while (1) {
        esp_err_t ret;
        ret = essl_wait_int(slave_handle, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "wait int error");
        }

        // uint32_t intr_raw, intr_st;
        // ret = essl_get_intr(slave_handle, NULL, NULL, portMAX_DELAY);
        // if (ret != ESP_OK) {
        //     ESP_LOGE(TAG, "wait intr error");
        // }
        // essl_clear_intr(slave_handle, intr_raw, portMAX_DELAY);
        while (1) {
            ret = essl_get_packet(slave_handle, read_buf, sizeof(read_buf), &buf_size, 1000);
            printf("%s", read_buf);
            memset(read_buf, 0, sizeof(read_buf));

            if (ret == ESP_OK) {
                break;
            }
        }

    }
}

void sdio_at_startup(void) {
    at_result_mutex = xSemaphoreCreateMutex();
    if (at_result_mutex == NULL) {
        ESP_LOGE(TAG, "create mutex failed");
    }

    ESP_ERROR_CHECK(sdio_power_on());
    ESP_ERROR_CHECK(uart_init(UART_NUM_0));
    ESP_ERROR_CHECK(sdio_slave_init(&slave_handle));
    ESP_ERROR_CHECK(sdio_slave_reset(slave_handle));

    xTaskCreate(sdio_receive_task, "at_receive", 4096, NULL, 20, NULL);

}

