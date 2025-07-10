idf_component_register(SRCS "at_sdio_cmd.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "driver" "esp_serial_slave_link")
