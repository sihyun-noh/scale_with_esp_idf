idf_component_register(SRCS "main.cpp" "gui.cpp" "utils.c" "sd_card.c" "ui_events.c" "ui.c" "ui_helpers.c" "hid_config.c" "file_manager.c" "control_task.c" "command.c"
                      INCLUDE_DIRS ".")

idf_build_set_property(COMPILE_DEFINITIONS "-DSENSOR_TYPE=-1" APPEND)
