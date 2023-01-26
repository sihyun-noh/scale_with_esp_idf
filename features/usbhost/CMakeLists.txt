set(req usb_host_msc usb vfs sysevent)

set(srcs)
set(includes)

if(CONFIG_USB_HOST_FEATURE)
  list(APPEND srcs "msc.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${includes}
                    REQUIRES ${req})
