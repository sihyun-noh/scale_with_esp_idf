cmake_minimum_required(VERSION 3.8)

#set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../../../package ${CMAKE_CURRENT_SOURCE_DIR}/../../../opensource/lvgl  ${CMAKE_CURRENT_SOURCE_DIR}/../../../opensource/esp_littlefs ${CMAKE_CURRENT_SOURCE_DIR}/../../../features ${CMAKE_CURRENT_SOURCE_DIR}/../../../arduino_platform/arduino)
set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../../../package ${CMAKE_CURRENT_SOURCE_DIR}/../../../opensource/lvgl  ${CMAKE_CURRENT_SOURCE_DIR}/../../../opensource/esp_littlefs ${CMAKE_CURRENT_SOURCE_DIR}/../../../features)

#configure_file(
#    ${CMAKE_CURRENT_SOURCE_DIR}/main/config.h.in
#    ${CMAKE_CURRENT_SOURCE_DIR}/sdkconfig.defaults
#)

# Pass the FW version to the source code to write the version string to flash (mfg_data).
set(FW_VERSION "${PROJECT_NAME}_${PROJECT_VERSION}-${GIT_COMMIT_HASH}-${PROJECT_STAGE}")
string(TOLOWER "${PROJECT_VERSION}-${GIT_COMMIT_HASH}-${PROJECT_STAGE}" PROJECT_VER)
add_definitions(-DFW_VERSION="${FW_VERSION}")
configure_file(version_config.h.in ${CMAKE_BINARY_DIR}/version_config.h)
include_directories(${CMAKE_BINARY_DIR})

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(rgb_panel)
