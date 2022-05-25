set(req nvs_flash spi_flash freertos log)
idf_component_register(SRCS "syscfg.c" "syscfg_cmd.c" "esp32/syscfg_impl.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req}
                    LDFRAGMENTS linker.lf)
