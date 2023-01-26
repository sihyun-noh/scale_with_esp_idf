set(req)

set(srcs)
set(includes)

if(CONFIG_UTILS_PACKAGE)
  list(APPEND srcs "utils.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
