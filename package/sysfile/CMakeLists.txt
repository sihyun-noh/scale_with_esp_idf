set(req spiffs esp_littlefs syslog esp_wifi)

set(srcs)
set(includes)

if(CONFIG_SMARTFARM_FILELOG_FEATURE)
  if(CONFIG_LITTLEFS_ENABLE)
    idf_build_set_property(COMPILE_DEFINITIONS "-DLITTLEFS_IMPL" APPEND)
    list(APPEND srcs
      "sysfile.c"
      "esp32/littlefs_impl.c")
  else()
    idf_build_set_property(COMPILE_DEFINITIONS "-DSPIFFS_IMPL" APPEND)
    list(APPEND srcs
      "sysfile.c"
      "esp32/spiffs_impl.c")
  endif()
  list(APPEND includes ".")
endif()

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
