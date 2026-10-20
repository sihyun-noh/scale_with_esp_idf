idf_component_register(
    SRCS "dajilog_system_core.c"
    INCLUDE_DIRS "include"
    REQUIRES dajilog_settings dajilog_ui_bridge dajilog_ui_msg
             dajilog_board_api dajilog_can dajilog_can_device_manager
             dajilog_can_profile
)
