set(SOURCES main.cpp led_task.c sensor_task.c led_strip_encoder.c usb_msc_task.c)
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS .)

