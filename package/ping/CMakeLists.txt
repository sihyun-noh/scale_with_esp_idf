set(req freertos lwip)

set(srcs)
set(includes)

if(CONFIG_PING_PACKAGE)
    list(APPEND srcs "icmp_echo_api.c" "icmp_echo_cmd.c" "esp32/icmp_echo_impl.c" "esp32/lwip_ping_impl.c")
    list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
    INCLUDE_DIRS ${includes}
    REQUIRES ${req}
    LDFRAGMENTS linker.lf)
