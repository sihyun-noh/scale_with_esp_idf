if(CONFIG_DAJILOG_UPDATE_PC_USB)
    idf_component_register(
        SRCS "dajilog_pc_usb_update.c"
        INCLUDE_DIRS "include"
        REQUIRES app_update esp_app_format esp_partition fatfs wear_levelling esp_tinyusb mbedtls
    )
else()
    idf_component_register()
endif()
