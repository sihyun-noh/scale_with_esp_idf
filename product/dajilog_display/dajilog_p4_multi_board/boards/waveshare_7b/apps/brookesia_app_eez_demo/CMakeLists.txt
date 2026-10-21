set(APP_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(APP_SOURCES_C "")
set(APP_SOURCES_CPP "")

if(CONFIG_DAJILOG_APP_EEZ_DEMO)
    file(GLOB_RECURSE APP_SOURCES_C CONFIGURE_DEPENDS "${APP_ROOT}/*.c")
    file(GLOB_RECURSE APP_SOURCES_CPP CONFIGURE_DEPENDS "${APP_ROOT}/*.cpp")
endif()

idf_component_register(
    SRCS ${APP_SOURCES_C} ${APP_SOURCES_CPP}
    INCLUDE_DIRS
        "."
        "eez/ui"
        "eez/ui/compat"
    REQUIRES
        dajilog_ui_adaptor
        dajilog_ui_bridge
        dajilog_ui_msg
        dajilog_system_core
    WHOLE_ARCHIVE
)

if(CONFIG_DAJILOG_APP_EEZ_DEMO)
    set_source_files_properties(
        ${APP_SOURCES_CPP}
        PROPERTIES COMPILE_FLAGS "-Wno-missing-field-initializers"
    )
endif()
