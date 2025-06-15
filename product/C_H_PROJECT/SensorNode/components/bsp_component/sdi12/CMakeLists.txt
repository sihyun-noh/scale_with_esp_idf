
set(req driver esp_timer freertos)
set(req_custom _nvs bsp_gpio)

idf_component_register(SRCS 
                        "SDI12.cpp" 
                        "sensor_auto_detect.c"
                        "sdi12_task.cpp" 
                       INCLUDE_DIRS 
                        "."
                       REQUIRES ${req} ${req_custom}

)

#set(KCONFIG_IN_FILENAME "Kconfig.in")
