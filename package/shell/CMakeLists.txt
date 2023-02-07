set(req freertos log console nvs_flash syscfg syslog mqttc ping sysfile wifi)

set(srcs)
set(includes)

if(CONFIG_SHELL_PACKAGE)
  list(APPEND srcs "shell_console.c" "esp32/shell_console_impl.c" "esp32/shell_command_impl.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
