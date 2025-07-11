#pragma once

typedef struct {
    SemaphoreHandle_t mutex;
    bool is_over;
    uint8_t *data;
}at_result_t;

essl_handle_t get_sdio_slave(void);
uint8_t *get_at_result(void);