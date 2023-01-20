set(req freertos log esp_event syscfg sysevent syslog setup wifi esp_wifi heap ping time)

set(srcs)
set(includes)

if(CONFIG_MONITOR_FEATURE)
  list(APPEND srcs
    "monitoring.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
