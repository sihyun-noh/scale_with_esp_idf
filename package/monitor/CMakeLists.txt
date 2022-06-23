set(req freertos log esp_event sysevent syslog esp_wifi heap)
idf_component_register(SRCS "monitoring.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req})
