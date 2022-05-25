set(req esp_wifi freertos log syslog sysevent)
idf_component_register(SRCS "wifi_manager.c" "esp32/wifi_manager_impl.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req}
                    LDFRAGMENTS linker.lf)
