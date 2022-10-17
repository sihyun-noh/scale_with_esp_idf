set(req nvs_flash spi_flash freertos log syslog json)
idf_component_register(SRCS "syscfg.c" "syscfg_cmd.c" "sys_config.c" "esp32/syscfg_impl.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req})

if(${PROJECT_TYPE} MATCHES SENSOR)
include_directories(INCLUDE_DIRS "../../product/sensor/main")
endif()
if(${PROJECT_TYPE} MATCHES ACTUATOR)
include_directories(INCLUDE_DIRS "../../product/actuator/main")
endif()
