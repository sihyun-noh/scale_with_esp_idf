set(req_esp esp_common esp_timer json)
idf_component_register(SRCS "data_table.c" "time_into_interval.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req_esp}
)
