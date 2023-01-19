if (IDF_VERSION_MAJOR GREATER_EQUAL 5)
  set(req esp-modbus syslog)
elseif (IDF_VERSION_MAJOR GREATER_EQUAL 4)
  set(req freemodbus syslog)
endif()

set(srcs)
set(includes)

if(CONFIG_MODBUS_PACKAGE)
  list(APPEND srcs "mb_master_rtu.c" "esp32/mb_master_impl.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
