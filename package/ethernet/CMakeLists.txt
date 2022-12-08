set(req freertos log esp_event syslog ethernet)
idf_component_register(SRCS "ethernet_manager.c" "esp32/ethernet_manager_impl.c"
    INCLUDE_DIRS "."
    REQUIRES ${req})
