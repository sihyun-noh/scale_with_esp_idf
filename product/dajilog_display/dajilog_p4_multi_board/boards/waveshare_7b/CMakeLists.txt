cmake_minimum_required(VERSION 3.16)

get_filename_component(WORKSPACE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(COMMON_COMPONENTS "${WORKSPACE_ROOT}/common/components")
set(BOARD_APPS "${CMAKE_CURRENT_LIST_DIR}/apps")
include("${WORKSPACE_ROOT}/cmake/dajilog_common_components.cmake")
set(EXTRA_COMPONENT_DIRS
    ${DAJILOG_COMMON_COMPONENT_DIRS}
    "${BOARD_APPS}/brookesia_app_eez_demo"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(dajilog_waveshare_p4_lcd_7b)

# Keep ESP-Hosted tasks in internal RAM while honoring the configured stack
# sizes. This is needed whenever the common Wi-Fi app is enabled for the 7B.
set(ESP_HOSTED_OS_HEADER
    "${CMAKE_SOURCE_DIR}/managed_components/espressif__esp_hosted/host/port/esp/freertos/include/port_esp_hosted_host_os.h"
)
if(EXISTS "${ESP_HOSTED_OS_HEADER}")
    file(READ "${ESP_HOSTED_OS_HEADER}" ESP_HOSTED_OS_CONTENT)
    string(REPLACE
        "#define RPC_TASK_STACK_SIZE                          (5*1024)"
        "#define RPC_TASK_STACK_SIZE                          CONFIG_ESP_HOSTED_RPC_TASK_STACK"
        ESP_HOSTED_OS_CONTENT "${ESP_HOSTED_OS_CONTENT}"
    )
    string(REPLACE
        "#define DFLT_TASK_STACK_SIZE                         (5*1024)"
        "#define DFLT_TASK_STACK_SIZE                         CONFIG_ESP_HOSTED_DFLT_TASK_STACK"
        ESP_HOSTED_OS_CONTENT "${ESP_HOSTED_OS_CONTENT}"
    )
    file(WRITE "${ESP_HOSTED_OS_HEADER}" "${ESP_HOSTED_OS_CONTENT}")
endif()
