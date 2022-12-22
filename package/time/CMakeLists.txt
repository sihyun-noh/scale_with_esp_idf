set(req freertos lwip esp_timer syslog)

set(srcs)
set(includes)

if(CONFIG_SMARTFARM_TIMEZONE_FEATURE)
  list(APPEND srcs "time_api.c" "uptime.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
