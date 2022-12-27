idf_component_register(SRCS "main.cpp" "adc.c" "utils.c" "sd_card.c" "ui_events.c" "ui.c" "ui_helpers.c" "hid_config.c"
                      INCLUDE_DIRS ".")

if(CONFIG_SMARTFARM_MQTT_FEATURE)
  idf_build_set_property(COMPILE_DEFINITIONS "-DSMARTFARM_ETHERNET_FEATURE" APPEND)
endif()

idf_build_set_property(COMPILE_DEFINITIONS "-DSENSOR_TYPE=-1" APPEND)
