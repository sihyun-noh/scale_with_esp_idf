set(req driver esp_eth esp_netif)

idf_component_register(SRCS "ethernet_driver.c" "ethernet_app.c"
                       REQUIRES ${req}
                       INCLUDE_DIRS ".")

# Kconfig.in 사용 선언
# set(KCONFIG_IN_FILENAME "Kconfig.in")
