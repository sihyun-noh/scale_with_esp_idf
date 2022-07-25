set(req freertos log esp_event syscfg sysevent syslog wifi esp_wifi heap ping)
idf_component_register(SRCS "monitoring.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req})
