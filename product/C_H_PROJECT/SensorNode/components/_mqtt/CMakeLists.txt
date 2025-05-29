set(req _console _nvs esp_event mqtt json)
idf_component_register(SRCS "console_mqtt.c" "mqtt_app.c" "mqtt_handler.c" "mqtt_publish.c"
                      INCLUDE_DIRS "."
                      REQUIRES ${req}
)
if(CONFIG_NVS_CMD_AUTO_REGISTRATION)
    target_link_libraries(${COMPONENT_LIB} PRIVATE "-u console_cmd_mqtt_register")
endif()
