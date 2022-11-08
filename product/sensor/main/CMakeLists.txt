idf_component_register(SRCS "main.c" "sensor_task.c" "mqtt_task.c" "adc.c" "utils.c" "ota_task.c"
                      INCLUDE_DIRS ".")

if(CONFIG_LED_FEATURE)
  target_sources(${COMPONENT_LIB} PRIVATE "led_task.c")
endif()
