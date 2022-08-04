# This is the project CMakeLists.txt file for the test subproject
cmake_minimum_required(VERSION 3.5)

# Include the components directory of the main application:
#
set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../package)

#set(TEST_COMPONENTS "testable" CACHE STRING "List of components to test" FORCE)
#set(TEST_COMPONENTS "wifi" CACHE STRING "List of components to test_wifi" FORCE)
set(TEST_COMPONENTS "syscfg" CACHE STRING "List of components to test_syscfg" FORCE)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(smartfarm-ief_test)
