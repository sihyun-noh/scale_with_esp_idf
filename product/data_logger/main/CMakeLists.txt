set(SOURCES main.cpp led_task.c sensor_task.c battery_task.c led_strip_rmt_sk68xx.c usb_msc_task.c)
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS .)

