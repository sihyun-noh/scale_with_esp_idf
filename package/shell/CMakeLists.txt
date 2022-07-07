set(req freertos log console nvs_flash syscfg syslog mqttc ping)
idf_component_register(SRCS "shell_console.c" "esp32/shell_console_impl.c" "esp32/shell_command_impl.c"
                    INCLUDE_DIRS "." 
                    REQUIRES ${req})
