idf_component_register(
    SRCS "dajilog_ui_adaptor.c"
    INCLUDE_DIRS "include"
    REQUIRES dajilog_ui_bridge dajilog_ui_msg freertos
)
