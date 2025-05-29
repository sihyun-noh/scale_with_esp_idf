
#include "nvs_cfg.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include <stdlib.h>

#define CFG_NAMESPACE "cfg_data"
static const char *TAG = "nvs_cfg";

esp_err_t cfg_nvs_init(void) {
  esp_err_t err = nvs_flash_init_partition("cfg");
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGW(TAG, "Erasing cfg partition due to lack of space or version mismatch.");
    ESP_ERROR_CHECK(nvs_flash_erase_partition("cfg"));
    err = nvs_flash_init_partition("cfg");
  }
  ESP_LOGI(TAG, "cfg_nvs_init result: %s", esp_err_to_name(err));
  return err;
}

esp_err_t cfg_set_u32(const char *key, uint32_t value) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open_from_partition("cfg", CFG_NAMESPACE, NVS_READWRITE, &handle);
  if (err != ESP_OK)
    return err;

  ESP_LOGD(TAG, "Save u32 %s = %lu", key, value);
  err = nvs_set_u32(handle, key, value);
  err = nvs_commit(handle);
  if (err != ESP_OK)
    ESP_LOGD(TAG, "Write %s = %lu Failed!", key, value);
  else
    ESP_LOGD(TAG, "Write %s = %lu Done", key, value);
  nvs_close(handle);
  return err;
}

esp_err_t cfg_get_u32(const char *key, uint32_t *value) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open_from_partition("cfg", CFG_NAMESPACE, NVS_READONLY, &handle);
  if (err != ESP_OK)
    return err;

  err = nvs_get_u32(handle, key, value);
  switch (err) {
    case ESP_OK: ESP_LOGD(TAG, "Read %s = %lu", key, *value); break;
    case ESP_ERR_NVS_NOT_FOUND: ESP_LOGD(TAG, "The value %s is not initialized yet!", key); break;
    default: ESP_LOGD(TAG, "Error (%s) reading!", esp_err_to_name(err));
  }
  nvs_close(handle);
  return err;
}

esp_err_t cfg_set_str(const char *key, const char *value) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open_from_partition("cfg", CFG_NAMESPACE, NVS_READWRITE, &handle);
  ESP_LOGD(TAG, "Save %s = %s", key, value);
  err = nvs_set_str(handle, key, value);
  err = nvs_commit(handle);
  if (err != ESP_OK)
    ESP_LOGD(TAG, "Write %s = %s Failed!", key, value);
  else
    ESP_LOGD(TAG, "Write %s = %s Done", key, value);
  nvs_close(handle);
  return err;
}

esp_err_t cfg_get_str(const char *key, char **value) {
  nvs_handle_t handle;
  size_t size;
  esp_err_t err = nvs_open_from_partition("cfg", CFG_NAMESPACE, NVS_READONLY, &handle);
  err = nvs_get_str(handle, key, NULL, &size);
  if (err != ESP_OK) {
    ESP_LOGD(TAG, "Error (%s) getting required size!", esp_err_to_name(err));
    nvs_close(handle);
    return err;
  }
  char *data = malloc(size);
  if (data == NULL) {
    ESP_LOGD(TAG, "Error allocating memory!");
    nvs_close(handle);
    return ESP_ERR_NO_MEM;
  }
  err = nvs_get_str(handle, key, data, &size);
  switch (err) {
    case ESP_OK: ESP_LOGD(TAG, "Read %s = %s", key, data); break;
    case ESP_ERR_NVS_NOT_FOUND: ESP_LOGD(TAG, "The value %s is not initialized yet!", key); break;
    default: ESP_LOGD(TAG, "Error (%s) reading!", esp_err_to_name(err));
  }
  if (err != ESP_OK) {
    free(data);
    nvs_close(handle);
    return err;
  }
  *value = data;
  nvs_close(handle);
  return err;
}

esp_err_t cfg_set_blob(const char *key, const void *data, size_t size) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open_from_partition("cfg", CFG_NAMESPACE, NVS_READWRITE, &handle);
  ESP_LOGD(TAG, "Save blob %s (%u bytes)", key, (unsigned)size);
  err = nvs_set_blob(handle, key, data, size);
  err = nvs_commit(handle);
  if (err != ESP_OK)
    ESP_LOGD(TAG, "Write %s Failed!", key);
  else
    ESP_LOGD(TAG, "Write %s Done", key);
  nvs_close(handle);
  return err;
}

esp_err_t cfg_get_blob(const char *key, void *data_out, size_t size) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open_from_partition("cfg", CFG_NAMESPACE, NVS_READONLY, &handle);
  size_t required_size = size;
  err = nvs_get_blob(handle, key, data_out, &required_size);
  switch (err) {
    case ESP_OK: ESP_LOGD(TAG, "Read blob %s (%u bytes)", key, (unsigned)required_size); break;
    case ESP_ERR_NVS_NOT_FOUND: ESP_LOGD(TAG, "The value %s is not initialized yet!", key); break;
    default: ESP_LOGD(TAG, "Error (%s) reading!", esp_err_to_name(err));
  }
  nvs_close(handle);
  return err;
}
