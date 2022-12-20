idf_component_register(SRCS "led_task.c" "main.c" "adc.c" "utils.c" "sd_card.c"
                      INCLUDE_DIRS ".")
idf_build_set_property(COMPILE_DEFINITIONS "-DSENSOR_TYPE=-1" APPEND)
