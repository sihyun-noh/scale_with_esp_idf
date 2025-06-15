set(req driver )
set(req_custom _nvs)

idf_component_register(SRCS 
                        "bsp_gpio.c" 
                       INCLUDE_DIRS 
                        "."
                       REQUIRES ${req} ${req_custom})
