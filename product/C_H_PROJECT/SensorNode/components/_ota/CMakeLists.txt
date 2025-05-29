set(req esp_http_client app_update esp_https_ota esp_netif mbedtls esp_psram)

idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS "ota_update.c" 
                       INCLUDE_DIRS "."
                       REQUIRES ${req}
                       EMBED_TXTFILES ${project_dir}/components/_ota/server_certs/ca_cert.pem
)

