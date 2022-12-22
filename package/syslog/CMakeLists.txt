set(req sysfile)

set(srcs)
set(includes)

if(CONFIG_SMARTFARM_SYSLOG_FEATURE)
    list(APPEND srcs "log.c" "syslog.c" "filelog.c")
    list(APPEND includes ".")
endif()

idf_component_register(
  SRCS ${srcs}
  INCLUDE_DIRS ${includes}
  REQUIRES ${req})
