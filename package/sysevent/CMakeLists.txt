set(req freertos log esp_event monitor)
idf_component_register(SRCS "sysevent.c" "esp32/sysevent_impl.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req})
