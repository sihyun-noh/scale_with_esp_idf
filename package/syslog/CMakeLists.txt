set(req sysfile)
idf_component_register(
  SRCS "log.c" "syslog.c" "filelog.c"
  INCLUDE_DIRS "."
  REQUIRES ${req})
