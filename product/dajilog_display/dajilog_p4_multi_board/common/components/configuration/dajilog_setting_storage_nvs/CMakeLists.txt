idf_component_register(
    SRCS "dajilog_setting_storage_nvs.c"
    INCLUDE_DIRS "include"
    REQUIRES freertos nvs_flash
)
