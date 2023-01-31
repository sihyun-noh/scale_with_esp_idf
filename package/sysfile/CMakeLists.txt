set(req spiffs esp_littlefs syslog)

set(srcs)
set(includes)

if(CONFIG_SYSFILE_PACKAGE)
  if(CONFIG_LITTLEFS_PACKAGE)
    list(APPEND srcs "esp32/littlefs_impl.c")
  elseif(CONFIG_SPIFFS_PACKAGE)
    list(APPEND srcs "esp32/spiffs_impl.c")
  endif()
  list(APPEND srcs "sysfile.c")
  list(APPEND includes "." "esp32")
endif()

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
