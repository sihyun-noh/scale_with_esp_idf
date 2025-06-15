

#include "esp_err.h"
#include "stdio.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "file_manager.h"

#define CHUNK_SIZE 1024  // 1KB씩 읽기
// #define SERVER_URL "http://192.168.50.12:1880/upload"
// #define SERVER_URL       "https://192.168.50.12:1880/upload"
#define SERVER_URL       "https://" CONFIG_SERVER_URL "/upload"
#define FILESTORAGE      "/storage"
#define READ_BUFFER_SIZE 128

#define MULTIPART_BOUNDARY "----ESP32Boundary"

#define FILENAME_ON_SERVER "example.txt"  // 서버에 보낼 파일 이름

static const char* TAG = "upload_file";
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

esp_err_t upload_file_multipart(const char* filepath, const char* filename);

#define TABLE_SIZE 10
static file_data_ctx_t storage_file_info;
static file_data_ctx_t storage_file_info_tb[TABLE_SIZE];

esp_err_t file_info_helper() {
  file_data_ctx_t* file_info_table = NULL;
  read_file_info(&storage_file_info);

  uint8_t nfiles = storage_file_info.nfiles;
  if (nfiles == 0) {
    ESP_LOGW(TAG, "No files to load");
    return ESP_OK;
  }

  read_file_info_table(&file_info_table);
  if (file_info_table == NULL) {
    ESP_LOGE(TAG, "file_info_table is NULL!");
    return ESP_FAIL;
  }

  if (nfiles > TABLE_SIZE) {
    ESP_LOGW(TAG, "Too many files (%d), truncating to %d", nfiles, TABLE_SIZE);
    nfiles = TABLE_SIZE;
  }

  ESP_LOGI(TAG, "Copying %d file entries", nfiles);
  for (int i = 0; i < nfiles; i++) {
    ESP_LOGW(TAG, "File[%d]: name=%s, size=%s", i, file_info_table[i].file_name, file_info_table[i].file_size);

    memcpy(storage_file_info_tb[i].file_name, file_info_table[i].file_name, sizeof(file_info_table[i].file_name));
    memcpy(storage_file_info_tb[i].file_size, file_info_table[i].file_size, sizeof(file_info_table[i].file_size));
  }

  return ESP_OK;
}

esp_err_t file_upload_proceed() {
  char filepath[300] = { 0 };

  ESP_LOGI(TAG, "Starting file upload process...");
  file_info_helper();  // 업데이트된 file_info_table -> storage_file_info_tb

  uint8_t file_count = storage_file_info.nfiles;
  if (file_count == 0) {
    ESP_LOGW(TAG, "No files to upload");
    return ESP_FAIL;
  }

  uint8_t success_count = 0;
  uint8_t fail_count = 0;

  while (file_count > 0) {
    file_count--;  // 인덱스 역방향 접근
    memset(filepath, 0, sizeof(filepath));
    snprintf(filepath, sizeof(filepath), "%s/%s", FILESTORAGE, storage_file_info_tb[file_count].file_name);

    ESP_LOGI(TAG, "Uploading file [%d]: %s", file_count, filepath);
    esp_err_t err = upload_file_multipart(filepath, storage_file_info_tb[file_count].file_name);

    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Upload failed for: %s", filepath);
      fail_count++;
      // 필요 시 실패한 파일 로그를 파일로 남기거나 continue로 다음 파일 시도 가능
      continue;
    }

    success_count++;
  }

  ESP_LOGI(TAG, "Upload process done. Success: %d, Fail: %d", success_count, fail_count);
  return (fail_count == 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t upload_file_multipart(const char* filepath, const char* filename) {
  FILE* file = fopen(filepath, "r");
  if (!file) {
    ESP_LOGE(TAG, "Failed to open file: %s", filepath);
    return ESP_FAIL;
  }

  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  // multipart header for file part
  char header_file[512];
  int header_file_len = snprintf(header_file, sizeof(header_file),
                                 "--" MULTIPART_BOUNDARY
                                 "\r\n"
                                 "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
                                 "Content-Type: text/plain\r\n\r\n",
                                 filename);

  // multipart header for filename part
  char header_filename[256];
  int header_filename_len = snprintf(header_filename, sizeof(header_filename),
                                     "\r\n--" MULTIPART_BOUNDARY
                                     "\r\n"
                                     "Content-Disposition: form-data; name=\"filename\"\r\n\r\n%s",
                                     filename);

  // multipart footer
  char footer[128];
  int footer_len = snprintf(footer, sizeof(footer), "\r\n--" MULTIPART_BOUNDARY "--\r\n");

  // total content length
  size_t total_size = header_file_len + file_size + header_filename_len + footer_len;

  esp_http_client_config_t config = {
    .url = SERVER_URL,
    .cert_pem = (char*)server_cert_pem_start,
  };
  esp_http_client_handle_t client = esp_http_client_init(&config);

  char content_length_str[32];
  snprintf(content_length_str, sizeof(content_length_str), "%d", (int)total_size);

  esp_http_client_set_method(client, HTTP_METHOD_POST);
  esp_http_client_set_header(client, "Content-Type", "multipart/form-data; boundary=" MULTIPART_BOUNDARY);
  esp_http_client_set_header(client, "Content-Length", content_length_str);

  esp_err_t err = esp_http_client_open(client, total_size);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    fclose(file);
    return ESP_FAIL;
  }

  // 1. Write header for file
  esp_http_client_write(client, header_file, header_file_len);

  // 2. Write file content
  char buffer[CHUNK_SIZE];
  size_t read_bytes;
  while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
    esp_http_client_write(client, buffer, read_bytes);
  }

  fclose(file);

  // 3. Write header for filename
  esp_http_client_write(client, header_filename, header_filename_len);

  // 4. Write footer
  esp_http_client_write(client, footer, footer_len);

  // Finalize
  esp_http_client_fetch_headers(client);
  int status_code = esp_http_client_get_status_code(client);
  esp_http_client_close(client);
  esp_http_client_cleanup(client);

  if (status_code != 200) {
    ESP_LOGE(TAG, "Upload failed, HTTP status: %d", status_code);
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "File uploaded successfully: %s", filepath);
  return ESP_OK;
}
