set(req freemodbus syslog)
idf_component_register(SRCS "mb_master_rtu.c" "esp32/mb_master_impl.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req})
