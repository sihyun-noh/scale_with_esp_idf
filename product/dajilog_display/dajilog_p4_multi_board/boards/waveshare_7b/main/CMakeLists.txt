idf_component_register(
    SRCS "main.cpp" "dajilog_product_controller.c"
         "dajilog_product_settings.c"
         "dajilog_motor_control.c" "dajilog_speed_sensor.c"
         "dajilog_product_catalog.c"
    INCLUDE_DIRS "." "${CMAKE_CURRENT_BINARY_DIR}"
    PRIV_REQUIRES dajilog_product_start dajilog_system_core
                  dajilog_calibration_catalog
                  dajilog_ridge_preview
                  dajilog_setting_table dajilog_setting_storage_nvs
                  dajilog_ui_bridge dajilog_ui_msg dajilog_board_api
                  dajilog_can dajilog_can_device_manager dajilog_can_profile
                  freertos esp_timer
)

# Compile the checked-in CSV into a read-only catalog. Import from USB is a
# separate future feature; CSV edits currently require a rebuild.
set(PRODUCT_CSV "${CMAKE_CURRENT_LIST_DIR}/../config/examples/PRODUCT_CATALOG_TEMPLATE.csv")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${PRODUCT_CSV}")
file(STRINGS "${PRODUCT_CSV}" PRODUCT_CSV_LINES ENCODING UTF-8)
list(POP_FRONT PRODUCT_CSV_LINES PRODUCT_CSV_HEADER)
if(NOT PRODUCT_CSV_HEADER STREQUAL "schema_version,product_code,product_name,nitrogen_percent,enabled")
    message(FATAL_ERROR "Product catalog CSV header is invalid")
endif()
list(LENGTH PRODUCT_CSV_LINES PRODUCT_CSV_COUNT)
if(PRODUCT_CSV_COUNT GREATER 8 OR PRODUCT_CSV_COUNT LESS 1)
    message(FATAL_ERROR "Product catalog must contain 1..8 entries")
endif()
set(PRODUCT_CATALOG_DATA "")
set(PRODUCT_CATALOG_CODES "")
foreach(PRODUCT_CSV_LINE IN LISTS PRODUCT_CSV_LINES)
    string(REPLACE "," ";" PRODUCT_FIELDS "${PRODUCT_CSV_LINE}")
    list(LENGTH PRODUCT_FIELDS PRODUCT_FIELD_COUNT)
    if(NOT PRODUCT_FIELD_COUNT EQUAL 5)
        message(FATAL_ERROR "Product CSV row must have exactly five columns: ${PRODUCT_CSV_LINE}")
    endif()
    list(GET PRODUCT_FIELDS 0 PRODUCT_VERSION)
    list(GET PRODUCT_FIELDS 1 PRODUCT_CODE)
    list(GET PRODUCT_FIELDS 2 PRODUCT_NAME)
    list(GET PRODUCT_FIELDS 3 PRODUCT_NITROGEN)
    list(GET PRODUCT_FIELDS 4 PRODUCT_ENABLED)
    if(NOT PRODUCT_VERSION STREQUAL "1" OR
       NOT PRODUCT_CODE MATCHES "^[A-Za-z0-9_.-]+$" OR
       PRODUCT_NAME MATCHES "\"" OR
       NOT PRODUCT_NITROGEN MATCHES "^[0-9]+(\\.[0-9]+)?$" OR
       NOT PRODUCT_ENABLED MATCHES "^[01]$")
        message(FATAL_ERROR "Product CSV row has invalid fields: ${PRODUCT_CSV_LINE}")
    endif()
    string(LENGTH "${PRODUCT_CODE}" PRODUCT_CODE_LENGTH)
    string(LENGTH "${PRODUCT_NAME}" PRODUCT_NAME_LENGTH)
    if(PRODUCT_CODE_LENGTH GREATER_EQUAL 40 OR PRODUCT_NAME_LENGTH GREATER_EQUAL 64 OR
       PRODUCT_NITROGEN GREATER 100)
        message(FATAL_ERROR "Product code/name is too long or nitrogen exceeds 100%: ${PRODUCT_CODE}")
    endif()
    list(FIND PRODUCT_CATALOG_CODES "${PRODUCT_CODE}" PRODUCT_DUPLICATE_INDEX)
    if(NOT PRODUCT_DUPLICATE_INDEX EQUAL -1)
        message(FATAL_ERROR "Duplicate product code: ${PRODUCT_CODE}")
    endif()
    list(APPEND PRODUCT_CATALOG_CODES "${PRODUCT_CODE}")
    string(APPEND PRODUCT_CATALOG_DATA "    { .code = \"${PRODUCT_CODE}\", .name = \"${PRODUCT_NAME}\", .nitrogen_percent = ${PRODUCT_NITROGEN}f, .enabled = ${PRODUCT_ENABLED} },\n")
endforeach()
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/dajilog_product_catalog_data.inc" "${PRODUCT_CATALOG_DATA}")
