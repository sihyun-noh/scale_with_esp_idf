idf_component_register(SRCS "main.cpp" "utils.c" "actuator.c" "func_key.c" "control_task.c" "battery_task.c"
                      INCLUDE_DIRS ".")

if(CONFIG_LED_FEATURE)
  target_sources(${COMPONENT_LIB} PRIVATE "led_task.c")
endif()
idf_build_set_property(COMPILE_DEFINITIONS "-DSENSOR_TYPE=-1" APPEND)
