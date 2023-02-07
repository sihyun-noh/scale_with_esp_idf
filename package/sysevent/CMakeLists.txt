set(req freertos log esp_event monitor)

set(srcs)
set(includes)

if(CONFIG_SYSEVENT_PACKAGE)
  list(APPEND srcs "sysevent.c" "esp32/sysevent_impl.c" "sys_status.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
