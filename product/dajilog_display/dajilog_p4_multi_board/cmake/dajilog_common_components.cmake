# Shared product components used by every DAJILOG board target.
# Keep each directory basename unchanged: ESP-IDF uses it as the component
# name referenced by REQUIRES/PRIV_REQUIRES.

set(DAJILOG_PLATFORM_COMPONENT_DIRS
    "${COMMON_COMPONENTS}/platform/dajilog_board_api"
    "${COMMON_COMPONENTS}/platform/dajilog_product_start"
)

set(DAJILOG_CONFIGURATION_COMPONENT_DIRS
    "${COMMON_COMPONENTS}/configuration/dajilog_setting_table"
    "${COMMON_COMPONENTS}/configuration/dajilog_setting_storage_nvs"
    "${COMMON_COMPONENTS}/configuration/dajilog_settings"
)

set(DAJILOG_PRODUCT_COMPONENT_DIRS
    "${COMMON_COMPONENTS}/product/dajilog_system_core"
)

set(DAJILOG_UI_COMPONENT_DIRS
    "${COMMON_COMPONENTS}/ui/dajilog_ui_msg"
    "${COMMON_COMPONENTS}/ui/dajilog_ui_bridge"
    "${COMMON_COMPONENTS}/ui/dajilog_ui_adaptor"
)

set(DAJILOG_COMMUNICATION_COMPONENT_DIRS
    "${COMMON_COMPONENTS}/communication/base/dajilog_comm"
    "${COMMON_COMPONENTS}/communication/can/dajilog_can_profile"
    "${COMMON_COMPONENTS}/communication/can/dajilog_can"
    "${COMMON_COMPONENTS}/communication/can/dajilog_can_device_manager"
)

set(DAJILOG_UPDATE_COMPONENT_DIRS
    "${COMMON_COMPONENTS}/update/usb/dajilog_pc_usb_update"
    "${COMMON_COMPONENTS}/update/usb/dajilog_usb_memory_update"
)

set(DAJILOG_BROOKESIA_COMPONENT_DIRS
    "${COMMON_COMPONENTS}/brookesia/core/brookesia_core"
    "${COMMON_COMPONENTS}/brookesia/apps/brookesia_app_hello"
    "${COMMON_COMPONENTS}/brookesia/apps/brookesia_app_wifi"
    "${COMMON_COMPONENTS}/brookesia/apps/brookesia_app_pc_usb_update"
    "${COMMON_COMPONENTS}/brookesia/apps/brookesia_app_usb_memory_update"
    "${COMMON_COMPONENTS}/brookesia/apps/brookesia_app_can_diagnostic"
)

set(DAJILOG_COMMON_COMPONENT_DIRS
    ${DAJILOG_PLATFORM_COMPONENT_DIRS}
    ${DAJILOG_CONFIGURATION_COMPONENT_DIRS}
    ${DAJILOG_PRODUCT_COMPONENT_DIRS}
    ${DAJILOG_UI_COMPONENT_DIRS}
    ${DAJILOG_COMMUNICATION_COMPONENT_DIRS}
    ${DAJILOG_UPDATE_COMPONENT_DIRS}
    ${DAJILOG_BROOKESIA_COMPONENT_DIRS}
)
