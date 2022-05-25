set(req driver freertos hal esp_adc_cal log pal)

set(srcs)
set(includes)

if(CONFIG_BATTERY_FEATURE)
  list(APPEND srcs 
    "battery/battery.c")
  list(APPEND includes "./battery")
endif()

if(CONFIG_SHT3x_FEATURE)
  list(APPEND srcs 
    "sht3x/sht3x.c")
  list(APPEND includes "./sht3x")
endif()

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
