set(req_esp esp_http_client lwip)
set(req_ch etl ethernet_driver sdi12 _wifi _console _nvs _ota _mqtt data_table sysfile)

idf_component_register(SRCS "main.cpp" "task_manager.c" "task_data_table.c" "task_upload_file.c" "time_sync.c" "sdi12_task.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES ${req_esp} ${req_ch}
)
