idf_component_register(
    SRCS "dajilog_product_start.cpp"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES dajilog_board_api dajilog_board_port dajilog_system_core brookesia_core app_update
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)
