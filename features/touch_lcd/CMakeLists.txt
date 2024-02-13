if(${IDF_TARGET} MATCHES "esp32s3")
set(req vfs esp_lcd lvgl sysevent log)
endif()

set(srcs)
set(includes)

if(CONFIG_LCD_RGB_16BIT_GT911_FEATURE)
  list(APPEND srcs "lcd_panel_rgb_gt911.c")
  list(APPEND includes ".")
  message("CONFIG_LCD_RGB_16BIAT_GT911_FEATURE")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
