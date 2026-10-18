idf_component_register(SRCS "dajilog_can.c" INCLUDE_DIRS "include" REQUIRES dajilog_can_profile driver)
target_compile_definitions(${COMPONENT_LIB} PRIVATE CONFIG_TWAI_SUPPRESS_DEPRECATE_WARN=1)
