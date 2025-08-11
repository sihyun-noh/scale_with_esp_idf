set(req_esp esp_http_client lwip)
set(req_custom ethernet_driver bsp_gpio sdi12 _wifi _console _nvs _ota _mqtt data_table sysfile)

idf_component_register(SRCS 
                    "main.cpp" 
                    "strategy_task_mgr.c" 
                    "strategy_task_mgr_sensor.c" 
                    "data_table_task.c" 
                    "upload_file_task.c" 
                    "time_sync.c" 
                    INCLUDE_DIRS "."
                    REQUIRES ${req_esp} ${req_custom}
                    EMBED_TXTFILES ${project_dir}/main/server_certs/ca_cert.pem
)
