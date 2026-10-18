idf_component_register(
    SRCS "dajilog_board_port.c"
    INCLUDE_DIRS "include"
    REQUIRES dajilog_board_api esp32_p4_wifi6_touch_lcd_5 esp_lvgl_adapter
)
