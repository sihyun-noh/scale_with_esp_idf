set(req esp_http_client spi_flash bootloader_support app_update syslog syscfg)

set(srcs)
set(includes)

if(CONFIG_OTA_FEATURE)
  list(APPEND srcs "ota_fw.c" "esp32/ota_fw_impl.c")
  list(APPEND includes ".")
endif()

idf_component_register(SRCS ${srcs}
  INCLUDE_DIRS ${includes}
  REQUIRES ${req})
