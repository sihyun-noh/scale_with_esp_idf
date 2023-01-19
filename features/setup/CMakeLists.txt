set(req freertos log mdns time syscfg syslog sysevent wifi esp_hw_support esp_https_server json ping monitor ethernet)
set(certs "certs/cacert.pem" "certs/prvtkey.pem")

if(CONFIG_EASYSETUP_FEATURE)
  if(CONFIG_ETHERNET_PACKAGE)
    list(APPEND srcs
      "ethernet_easy_setup.c" "mdns.c")
  else()
    list(APPEND srcs
      "easy_setup.c" "mdns.c")
  endif()
endif()

idf_component_register(SRCS "${srcs}"
    INCLUDE_DIRS "."
    REQUIRES ${req}
    EMBED_TXTFILES ${certs})
