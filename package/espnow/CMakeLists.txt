set(req log wifi esp_wifi)

set(srcs)
set(includes)

if(CONFIG_SMARTFARM_ESPNOW_FEATURE)
  list(APPEND srcs "espnow.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS "${srcs}"
    INCLUDE_DIRS ${includes}
    REQUIRES ${req})

