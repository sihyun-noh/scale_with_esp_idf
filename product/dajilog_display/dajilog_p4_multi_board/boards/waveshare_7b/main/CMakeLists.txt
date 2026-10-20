idf_component_register(
    SRCS "main.cpp" "dajilog_product_controller.c"
         "dajilog_motor_control.c" "dajilog_speed_sensor.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES dajilog_product_start dajilog_system_core dajilog_settings
                  dajilog_ui_bridge dajilog_ui_msg dajilog_board_api
                  dajilog_can dajilog_can_device_manager dajilog_can_profile
                  freertos esp_timer
)
