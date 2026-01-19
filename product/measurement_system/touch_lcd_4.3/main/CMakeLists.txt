

# idf_component_register(SRCS "LCD_Wifi_Manager.cpp" "main.cpp" "lvgl_demo_ui.cpp" "ap_mode_httpserver.c" 
#                             "dpp_enrollee.c" "LCD_Wifi_Manager.cpp" "wifiscan.c" "mqtt.c" ${SOURCES}
#                             # 480_800_16_bmp.c
#                        INCLUDE_DIRS "." )


# idf_component_register(SRCS "main.cpp" "lvgl_demo_ui.cpp" "ap_mode_httpserver.c" 
#                              "dpp_enrollee.c" "wifiscan.c" "mqtt.c" "ui/ui.c" "ui/ui_helpers.c" "ui/ui_events.c" "ui/screens/ui_Screen1.c" 
#                              "ui/images/ui_img_77785195.c" "ui/images/ui_img_2105472862.c"
#                              "ui/components/ui_comp.c" "CAN_comn.c" 
#                              # 480_800_16_bmp.c
#                         INCLUDE_DIRS "." )
#
idf_component_register(
    SRCS
        "main.cpp"
        "lvgl_demo_ui.cpp"
        "ap_mode_httpserver.c"
        "dpp_enrollee.c"
        "wifiscan.c"
        "mqtt.c"
        "CAN_comn.c"

        # UI core
        "ui/ui.c"
        "ui/ui_helpers.c"
        "ui/ui_events.c"

        # UI screens
        "ui/screens/ui_Screen1.c"

        # UI components
        "ui/components/ui_comp.c"

        # UI images
        "ui/images/ui_img_77785195.c"
        "ui/images/ui_img_2105472862.c"

        # (optional)
        # "480_800_16_bmp.c"
    INCLUDE_DIRS
        "."
        "ui"
        "ui/screens"
        "ui/components"
        "ui/images"
)
