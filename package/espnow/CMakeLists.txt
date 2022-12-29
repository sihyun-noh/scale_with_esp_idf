set(req log wifi esp_wifi)

set(srcs)
set(includes)

list(APPEND srcs "espnow.c")
list(APPEND includes ".")

idf_component_register(SRCS "${srcs}"
    INCLUDE_DIRS ${includes}
    REQUIRES ${req})
