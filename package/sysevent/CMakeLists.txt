set(req freertos log esp_event)
idf_component_register(SRCS "sysevent.c" "esp32/sysevent_impl.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req}
                    LDFRAGMENTS linker.lf)
