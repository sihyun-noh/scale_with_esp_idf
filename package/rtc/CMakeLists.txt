if(CONFIG_SMARTFARM_RTC_DS3231_FEATURE)
set(req syslog pal)
idf_component_register(
    SRCS "ds3231/ds3231.c"
    INCLUDE_DIRS "./ds3231"
    REQUIRES ${req})
endif()
