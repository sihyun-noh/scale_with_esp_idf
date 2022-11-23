set(req driver freertos hal esp_adc syslog pal)

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

if(CONFIG_SCD4x_FEATURE)
  list(APPEND srcs
    "scd4x/scd4x.c")
  list(APPEND includes "./scd4x")
endif()

if(CONFIG_ATLAS_PH_FEATURE)
  list(APPEND srcs
    "atlas_ph/atlas_ph.c")
  list(APPEND includes "./atlas_ph")
endif()

if(CONFIG_ATLAS_EC_FEATURE)
  list(APPEND srcs
    "atlas_ec/atlas_ec.c")
  list(APPEND includes "./atlas_ec")
endif()

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
