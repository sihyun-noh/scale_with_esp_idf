set(req freertos log esp_event esp_eth syslog esp_netif)

set(srcs)
set(includes)

if(CONFIG_ETHERNET_PACKAGE)
    list(APPEND srcs "ethernet_manager.c" "esp32/ethernet_manager_impl.c")
    list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
    INCLUDE_DIRS ${includes}
    REQUIRES ${req})
