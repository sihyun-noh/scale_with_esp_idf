if(CONFIG_SMARTFARM_ETHERNET_FEATURE)
    idf_build_set_property(COMPILE_DEFINITIONS "-DCONFIG_SMARTFARM_ETHERNET" APPEND)
endif()

idf_component_register(SRCS "main.c" "actuator_task.c" "actuator_mqtt_task.c" "ota_task.c"
    INCLUDE_DIRS ".")
