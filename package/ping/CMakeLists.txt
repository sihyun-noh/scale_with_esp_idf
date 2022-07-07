set(req freertos log lwip)
idf_component_register(SRCS "icmp_echo_api.c" "icmp_echo_cmd.c" "esp32/icmp_echo_impl.c"
                      INCLUDE_DIRS "."
                      REQUIRES ${req})
