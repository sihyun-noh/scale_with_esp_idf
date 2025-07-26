set(req nvs_flash _console json sdi12)
idf_component_register(SRCS "nvs_ext.c" "console_nvs.c" "nvs_cfg.c" "sensor_cfg_config.c"
                      INCLUDE_DIRS "."
                      REQUIRES ${req}
)
if(CONFIG_NVS_CMD_AUTO_REGISTRATION)
    target_link_libraries(${COMPONENT_LIB} PRIVATE "-u console_cmd_nvs_register")
endif()
