SET(SOURCES components/ui_comp_screen2_keyboard1.c
    components/ui_comp.c
    screens/ui_device_id_reg_Screen.c
    screens/ui_list_select_Screen.c
    screens/ui_main_Screen.c
    screens/ui_mode_2_Screen.c
    screens/ui_mode_1_main_Screen.c
    screens/ui_mode_1_main_set_reg_Screen.c
    screens/ui_time_set_Screen.c
    screens/ui_indicator_model_select_Screen.c
    screens/ui_version_screen.c
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
idf_component_register(SRCS "scale_read_485.c" "main.cpp" "gui.cpp" "sd_card.c" "ui.c" "ui_helpers.c" 
                            "file_manager.c" "sensor_task.c" "usb_msc_task.c" "control_task.c" "model_define.c"
                            "LIFO_stack.c" "i2s_speaker.c" "audio/volume.c" "screens/msg_common.c" "usb_msc_ota.c" ${SOURCES}
                      INCLUDE_DIRS ".")
