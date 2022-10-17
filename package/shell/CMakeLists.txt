set(req freertos log console nvs_flash syscfg syslog mqttc ping sysfile)
idf_component_register(SRCS "shell_console.c" "esp32/shell_console_impl.c" "esp32/shell_command_impl.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req})

if(${PROJECT_TYPE} MATCHES SENSOR)
include_directories(INCLUDE_DIRS "../../product/sensor/main")
endif()
if(${PROJECT_TYPE} MATCHES ACTUATOR)
include_directories(INCLUDE_DIRS "../../product/actuator/main")
endif()
