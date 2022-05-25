set(req mqtt freertos log)
idf_component_register(SRCS "mqtt.c" "esp32/mqtt_impl.c" "test/mqtt_client.c"
                      INCLUDE_DIRS "."
                      REQUIRES ${req})
