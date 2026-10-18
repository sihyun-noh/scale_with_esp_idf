if(CONFIG_DAJILOG_APP_HELLO)
    idf_component_register(
        SRCS "hello_app.cpp"
        INCLUDE_DIRS "."
        WHOLE_ARCHIVE
    )
else()
    idf_component_register()
endif()
