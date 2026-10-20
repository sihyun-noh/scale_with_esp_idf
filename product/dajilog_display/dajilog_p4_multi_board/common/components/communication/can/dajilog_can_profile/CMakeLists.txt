set(PROFILE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../can_profiles")
set(GENERATOR "${CMAKE_CURRENT_LIST_DIR}/../../../../tools/generate_can_profiles.py")
set(DEVICE_MANIFEST "${PROJECT_DIR}/config/can_devices.yaml")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(GENERATED_C "${GENERATED_DIR}/dajilog_can_profiles_generated.c")
set(GENERATED_H "${GENERATED_DIR}/dajilog_can_profiles_generated.h")
file(MAKE_DIRECTORY "${GENERATED_DIR}")
execute_process(
    COMMAND "${PYTHON}" "${GENERATOR}"
        --protocol-dir "${PROFILE_ROOT}/protocols"
        --devices "${DEVICE_MANIFEST}"
        --out-c "${GENERATED_C}"
        --out-h "${GENERATED_H}"
    RESULT_VARIABLE CAN_PROFILE_RESULT
    OUTPUT_VARIABLE CAN_PROFILE_STDOUT
    ERROR_VARIABLE CAN_PROFILE_STDERR
)
if(NOT CAN_PROFILE_RESULT EQUAL 0)
    message(FATAL_ERROR "CAN profile generation failed: ${CAN_PROFILE_STDERR}")
endif()
idf_component_register(
    SRCS "dajilog_can_tr07e.c" "dajilog_can_moonwalker.c" "${GENERATED_C}"
    INCLUDE_DIRS "include" "${GENERATED_DIR}"
)
