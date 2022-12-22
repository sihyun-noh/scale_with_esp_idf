set(req mbedtls syslog)

set(srcs)
set(includes)

if(CONFIG_LWMQTT_ENABLED)
    list(APPEND srcs
        "lwmqttc.c"
        "mqttc.c"
        "tls_lwmqttc.c"
        "lwmqtt/src/client.c"
        "lwmqtt/src/helpers.c"
        "lwmqtt/src/packet.c"
        "lwmqtt/src/string.c")
    list(APPEND includes "." "lwmqtt/include")
endif()

idf_component_register(SRCS ${srcs}
    INCLUDE_DIRS ${includes}
    REQUIRES ${req})
