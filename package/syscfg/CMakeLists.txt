set(req nvs_flash spi_flash freertos log syslog json)

set(srcs)
set(includes)

if(CONFIG_SYSCFG_PACKAGE)
  list(APPEND srcs "syscfg.c" "syscfg_cmd.c" "sys_config.c" "esp32/syscfg_impl.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
