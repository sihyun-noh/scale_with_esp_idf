set(req spiffs esp_littlefs syslog)

set(srcs)
set(includes)

if(CONFIG_SYSFILE_PACKAGE)
  if(CONFIG_LITTLEFS_PACKAGE)
    list(APPEND srcs
      "sysfile.c"
      "esp32/littlefs_impl.c")
  else()
    list(APPEND srcs
      "sysfile.c"
      "esp32/spiffs_impl.c")
  endif()
  list(APPEND includes ".")
endif()

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
