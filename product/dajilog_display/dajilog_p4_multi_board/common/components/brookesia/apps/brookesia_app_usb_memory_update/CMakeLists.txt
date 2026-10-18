if(CONFIG_DAJILOG_UPDATE_USB_MEMORY)
    idf_component_register(
        SRCS "usb_memory_update_app.cpp"
        INCLUDE_DIRS "."
        REQUIRES dajilog_usb_memory_update
        WHOLE_ARCHIVE
    )
else()
    idf_component_register()
endif()
