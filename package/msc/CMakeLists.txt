if(${IDF_TARGET} MATCHES "esp32s3")
idf_component_register(SRCS "msc.c"
                    INCLUDE_DIRS "."
                    REQUIRES usb usb_host_msc vfs)
endif()
