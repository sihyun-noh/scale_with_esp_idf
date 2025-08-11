#set(req esp_littlefs)
set(req joltwallet__littlefs)

set(srcs)
set(includes)

if(CONFIG_SYSFILE_PACKAGE)
  if(CONFIG_LITTLEFS_PACKAGE)
    list(APPEND srcs "littlefs_impl.c")
  endif()
  list(APPEND srcs "sys_file.c" "file_manager.c" "file_log.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
