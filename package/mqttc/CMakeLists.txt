set(req mqtt freertos log)

set(srcs)
set(includes)

if(CONFIG_MQTTC_PACKAGE)
  list(APPEND srcs "mqtt.c" "esp32/mqtt_impl.c" "test/mqtt_client.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
                      INCLUDE_DIRS ${includes}
                      REQUIRES ${req})
