set(req log wifi esp_wifi)

set(srcs)
set(includes)

if(CONFIG_ESPNOW_PACKAGE)
  list(APPEND srcs "espnow.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS "${srcs}"
    INCLUDE_DIRS ${includes}
    REQUIRES ${req})

