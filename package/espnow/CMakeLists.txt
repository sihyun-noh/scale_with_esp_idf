set(req freertos log esp_event syscfg sysevent syslog setup wifi esp_wifi heap ping time)

set(srcs)
set(includes)

list(APPEND srcs
  "espnow.c")
list(APPEND includes ".")

idf_component_register(SRCS "${srcs}"
  INCLUDE_DIRS ${includes}
  REQUIRES ${req})
