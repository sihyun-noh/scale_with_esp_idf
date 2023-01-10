set(req nvs_flash spi_flash freertos log syslog json)

set(srcs)
set(includes)

if(CONFIG_SMARTFARM_SYSCFG_FEATURE)
  list(APPEND srcs "syscfg.c" "syscfg_cmd.c" "sys_config.c" "esp32/syscfg_impl.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})

if(${PROJECT_TYPE} MATCHES SENSOR)
include_directories(INCLUDE_DIRS "../../product/sensor/main")
endif()
if(${PROJECT_TYPE} MATCHES ACTUATOR)
include_directories(INCLUDE_DIRS "../../product/actuator/main")
endif()
if(${PROJECT_TYPE} MATCHES LOGGER)
include_directories(INCLUDE_DIRS "../../product/data_logger/main")
endif()
if(${PROJECT_TYPE} MATCHES IRRIGATION_MASTER)
include_directories(INCLUDE_DIRS "../../product/irrigation/master/main")
endif()
if(${PROJECT_TYPE} MATCHES IRRIGATION_CHILD)
include_directories(INCLUDE_DIRS "../../product/irrigation/child/main")
endif()
if(${PROJECT_TYPE} MATCHES IRRIGATION_HID)
include_directories(INCLUDE_DIRS "../../product/irrigation/hid/main")
endif()
