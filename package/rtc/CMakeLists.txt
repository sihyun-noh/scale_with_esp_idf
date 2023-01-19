set(req syslog pal)

set(srcs)
set(includes)

if(CONFIG_RTC_DS3231_PACKAGE)
    list(APPEND srcs "ds3231/ds3231.c")
    list(APPEND includes "./ds3231")
endif()

idf_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS ${includes}
    REQUIRES ${req})
