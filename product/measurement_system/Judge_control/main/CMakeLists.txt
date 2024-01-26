SET(SOURCES components/ui_comp_screen2_keyboard1.c
    components/ui_comp.c
    screens/ui_device_id_reg_Screen.c
    screens/ui_list_select_Screen.c
    screens/ui_main_Screen.c
    screens/ui_mode_2_Screen.c
    screens/ui_Screen1.c
    screens/ui_Screen2.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    # ui_events.c
    fonts/ui_font_Display16.c
    fonts/ui_font_Display24.c
    fonts/ui_font_Display40.c
    fonts/ui_font_Display90.c
    fonts/NanumBar16.c
    fonts/NanumBar18.c
    fonts/NanumBar24.c
    fonts/NanumBar32.c)
idf_component_register(SRCS "uart_485.c" "main.cpp" "gui.cpp" "sd_card.c" "ui.c" "ui_helpers.c" "hid_config.c"
                            "file_manager.c" "control_task.c" "command.c" "sensor_task.c" "usb_msc_task.c" 
                            "i2s_speaker.c" "audio/volume.c" ${SOURCES}
                      INCLUDE_DIRS ".")
