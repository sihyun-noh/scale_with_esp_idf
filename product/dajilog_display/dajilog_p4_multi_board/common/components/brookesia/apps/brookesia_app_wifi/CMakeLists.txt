if(CONFIG_DAJILOG_APP_WIFI)
    idf_component_register(
        SRCS "wifi_app.cpp"
        INCLUDE_DIRS "."
        WHOLE_ARCHIVE
    )
else()
    idf_component_register()
endif()
