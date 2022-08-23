set(req esp_http_client spi_flash bootloader_support app_update syslog)
idf_component_register(SRCS "ota_fw.c" "esp32/ota_fw_impl.c"
  INCLUDE_DIRS "."
  REQUIRES ${req})
