if(CONFIG_DAJILOG_UPDATE_USB_MEMORY)
    idf_component_register(
        SRCS "dajilog_usb_memory_update.c"
        INCLUDE_DIRS "include"
        REQUIRES app_update esp_app_format esp_msc_ota usb_host_msc usb
    )
else()
    idf_component_register()
endif()
