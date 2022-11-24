set(req freertos log mdns time syscfg syslog sysevent wifi esp_hw_support esp_https_server json ping monitor ethernet)
set(certs "certs/cacert.pem" "certs/prvtkey.pem")
idf_component_register(SRCS "easy_setup.c" "mdns.c" "ethernet_easy_setup.c"
    INCLUDE_DIRS "."
    REQUIRES ${req}
    EMBED_TXTFILES ${certs})
