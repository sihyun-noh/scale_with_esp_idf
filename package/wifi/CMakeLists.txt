set(req esp_wifi freertos log syslog sysevent)

set(srcs)
set(includes)

if(CONFIG_WIFILIB_PACKAGE)
  list(APPEND srcs "wifi_manager.c" "esp32/wifi_manager_impl.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req}
                    LDFRAGMENTS linker.lf)
