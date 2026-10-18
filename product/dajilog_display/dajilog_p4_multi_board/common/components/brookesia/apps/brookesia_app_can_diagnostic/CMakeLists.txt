set(APP_SOURCES "")
if(CONFIG_DAJILOG_APP_CAN_DIAGNOSTIC)
    list(APPEND APP_SOURCES "can_diagnostic_app.cpp")
endif()

idf_component_register(
    SRCS ${APP_SOURCES}
    INCLUDE_DIRS "."
    REQUIRES brookesia_core lvgl__lvgl dajilog_board_api dajilog_can dajilog_can_device_manager dajilog_can_profile
    WHOLE_ARCHIVE
)
