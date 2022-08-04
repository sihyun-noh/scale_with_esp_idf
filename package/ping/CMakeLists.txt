set(req freertos lwip)
idf_component_register(SRCS "icmp_echo_api.c" "icmp_echo_cmd.c" "esp32/icmp_echo_impl.c" "esp32/lwip_ping_impl.c"
    INCLUDE_DIRS "."
    REQUIRES ${req}
    LDFRAGMENTS linker.lf)
