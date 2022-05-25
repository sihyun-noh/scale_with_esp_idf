set(req syslog)
idf_component_register(SRCS "time_api.c" "uptime.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req})
