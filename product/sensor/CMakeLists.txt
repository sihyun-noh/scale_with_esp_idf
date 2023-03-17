cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

set(COMPONENT_REQUIRES mbedtls)
# Set SmartFarm IoT framework component package
set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../../package ${CMAKE_CURRENT_SOURCE_DIR}/../../opensource ${CMAKE_CURRENT_SOURCE_DIR}/../../features)
set(EXCLUDE_COMPONENTS "LovyanGFX lvgl")

# Set project version
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 2)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(PROJECT_VERSION_FILE "${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}")

# Set project stage : "EVT", "DVT", "PVT"
set(PROJECT_STAGE "DVT")

# Set project name
set(PROJECT_NAME "SENS_$ENV{CURRENT_PROJECT}")
message("project name : ${PROJECT_NAME}")
set(PROJECT_TYPE "SENSOR")

# Get the current commit hash
execute_process(
    COMMAND git describe --abbrev=6 --always
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# Pass the FW version to the source code to write the version string to flash (mfg_data).
set(FW_VERSION "${PROJECT_NAME}_${PROJECT_VERSION}-${GIT_COMMIT_HASH}-${PROJECT_STAGE}")
add_definitions(-DFW_VERSION="${FW_VERSION}")
configure_file(version_config.h.in ${CMAKE_BINARY_DIR}/version_config.h)
include_directories(${CMAKE_BINARY_DIR})

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(SENSOR)
