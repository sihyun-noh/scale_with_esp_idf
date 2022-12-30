idf_component_register(SRCS "main.cpp" "adc.c" "utils.c" "actuator.c" "input_task.c" "control_task.c" "water_flow.c"
                      INCLUDE_DIRS ".")

if(CONFIG_LED_FEATURE)
  target_sources(${COMPONENT_LIB} PRIVATE "led_task.c")
endif()
idf_build_set_property(COMPILE_DEFINITIONS "-DSENSOR_TYPE=-1" APPEND)
