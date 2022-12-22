set(req freemodbus syslog)

set(srcs)
set(includes)

if(CONFIG_SMARTFARM_MODBUS_FEATURE)
  list(APPEND srcs "mb_master_rtu.c" "esp32/mb_master_impl.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
