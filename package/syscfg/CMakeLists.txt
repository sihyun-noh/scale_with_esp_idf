set(req nvs_flash spi_flash freertos log syslog json)
idf_component_register(SRCS "syscfg.c" "syscfg_cmd.c" "sys_config.c" "esp32/syscfg_impl.c"
                    INCLUDE_DIRS "." "../../product/sensor/main"
                    REQUIRES ${req})
