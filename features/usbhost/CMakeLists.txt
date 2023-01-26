if(${IDF_TARGET} MATCHES "esp32s3")
set(req usb_host_msc usb vfs sysevent)
endif()

set(srcs)
set(includes)

if(CONFIG_USB_HOST_FEATURE)
  list(APPEND srcs "msc.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
