set(req vfs)
set(srcs)
set(includes)

if(CONFIG_FILECOPY_PACKAGE)
  list(APPEND srcs "file_copy.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
