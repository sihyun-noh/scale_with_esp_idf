set(req freertos lwip esp_timer syslog syscfg rtc)

set(srcs)
set(includes)

if(CONFIG_TIMEZONE_PACKAGE)
  list(APPEND srcs "time_api.c" "uptime.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
