idf_component_register(
    SRCS
        "dajilog_settings.c"
        "dajilog_setting_schema.c"
    INCLUDE_DIRS "include"
    REQUIRES freertos nvs_flash
)
