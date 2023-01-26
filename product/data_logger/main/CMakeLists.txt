if(CONFIG_SMARTFARM_MQTT_FEATURE)
  idf_build_set_property(COMPILE_DEFINITIONS "-DCONFIG_SMARTFARM_MQTT" APPEND)
endif()

set(SOURCES main.cpp file_copy.c utils.c led_task.c sensor_task.c battery_task.c
  led_strip_rmt_sk68xx.c usb_msc_task.c)
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS .)

