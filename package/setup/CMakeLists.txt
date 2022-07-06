set(req freertos log mdns time syscfg syslog sysevent wifi esp_https_server json)
set(certs "certs/cacert.pem" "certs/prvtkey.pem")
idf_component_register(SRCS "easy_setup.c" "mdns.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req}
                    EMBED_TXTFILES ${certs})
