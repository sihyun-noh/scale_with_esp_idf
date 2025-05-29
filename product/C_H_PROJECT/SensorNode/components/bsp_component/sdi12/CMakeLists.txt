
set(req driver esp_timer freertos)

idf_component_register(SRCS 
                        "SDI12.cpp" 
                        "sensor_auto_detect.c"
                       #"sdi12_bitbang.c" 
                       INCLUDE_DIRS 
                        "."
                       REQUIRES ${req} 

)

#set(KCONFIG_IN_FILENAME "Kconfig.in")
