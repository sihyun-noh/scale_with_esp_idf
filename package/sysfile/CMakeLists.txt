set(req freertos log esp_event spiffs sysevent syslog esp_wifi heap ping)

set(srcs)
set(includes)

if(CONFIG_SMARTFARM_FILELOG_FEATURE)
  list(APPEND srcs
    "sysfile.c"
    "esp32/spiffs_impl.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
