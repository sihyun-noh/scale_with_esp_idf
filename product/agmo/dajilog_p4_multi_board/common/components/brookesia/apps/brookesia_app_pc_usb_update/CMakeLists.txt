if(CONFIG_DAJILOG_UPDATE_PC_USB)
    idf_component_register(
        SRCS "pc_usb_update_app.cpp"
        INCLUDE_DIRS "."
        REQUIRES dajilog_pc_usb_update
        WHOLE_ARCHIVE
    )
else()
    idf_component_register()
endif()
