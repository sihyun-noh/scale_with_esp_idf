set(req syslog sysfile)

set(srcs)
set(includes)

if(CONFIG_SYSLOG_PACKAGE)
    list(APPEND srcs "filelog.c")
    list(APPEND includes ".")
endif()

idf_component_register(
  SRCS ${srcs}
  INCLUDE_DIRS ${includes}
  REQUIRES ${req})
