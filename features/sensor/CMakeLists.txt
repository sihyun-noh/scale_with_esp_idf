set(req driver freertos hal esp_adc syslog pal)

set(srcs)
set(includes)

if(CONFIG_SHT3X_FEATURE)
  list(APPEND srcs
    "sht3x/sht3x.c")
  list(APPEND includes "./sht3x")
endif()

if(CONFIG_SCD4X_FEATURE)
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

if(CONFIG_WINSEN_ZE03_FEATURE)
  list(APPEND srcs
    "winsen_ze03/winsen_ze03.c")
  list(APPEND includes "./winsen_ze03")
endif()

if(CONFIG_WINSEN_ZCE04B_FEATURE)
  list(APPEND srcs
    "winsen_zce04b/winsen_zce04b.c")
  list(APPEND includes "./winsen_zce04b")
endif()

if(CONFIG_TH01_FEATURE)
  list(APPEND srcs
    "th01/th01.c")
  list(APPEND includes "./th01")
endif()

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
