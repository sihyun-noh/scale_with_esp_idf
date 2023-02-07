set(req driver freertos hal syslog pal)

set(srcs)
set(includes)

if(CONFIG_LED_PACKAGE)
  list(APPEND srcs
    "led/led.c")
  list(APPEND includes "./led")
endif()

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
