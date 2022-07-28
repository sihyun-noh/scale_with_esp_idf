set(req freertos log esp_event spiffs sysevent syslog esp_wifi heap ping)
idf_component_register(SRCS "esp32/spiffs_impl.c" "sysfile.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req})
