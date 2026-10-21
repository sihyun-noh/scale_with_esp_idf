#include "dajilog_setting_storage_nvs.h"

#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "nvs.h"
#include "nvs_flash.h"

#define STORAGE_RECORD_MAGIC 0x44535432U
#define NVS_KEY_MAX_LENGTH   15U

typedef struct {
    uint32_t magic;
    uint32_t schema_version;
    uint32_t payload_size;
    uint32_t checksum;
} storage_record_header_t;

static uint32_t payload_checksum(const void *payload, size_t payload_size)
{
    const uint8_t *bytes = (const uint8_t *)payload;
    uint32_t hash = 2166136261U;
    for (size_t i = 0; i < payload_size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619U;
    }
    return hash;
}

static bool storage_ready(const dajilog_setting_storage_nvs_t *storage)
{
    return storage != NULL && storage->initialized && storage->lock != NULL;
}

static bool key_valid(const char *key)
{
    if (key == NULL || key[0] == '\0') {
        return false;
    }
    const size_t length = strnlen(key, NVS_KEY_MAX_LENGTH + 1U);
    return length > 0U && length <= NVS_KEY_MAX_LENGTH;
}

esp_err_t dajilog_setting_storage_nvs_init(
    dajilog_setting_storage_nvs_t *storage,
    const char *namespace_name)
{
    if (storage == NULL || namespace_name == NULL || namespace_name[0] == '\0') {
        return ESP_ERR_INVALID_ARG;
    }
    if (storage->initialized) {
        return ESP_OK;
    }
    const size_t length = strnlen(namespace_name,
                                  DAJILOG_NVS_NAMESPACE_CAPACITY);
    if (length == 0U || length >= DAJILOG_NVS_NAMESPACE_CAPACITY) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t error = nvs_flash_init();
    if (error != ESP_OK) {
        return error;
    }
    SemaphoreHandle_t lock = xSemaphoreCreateMutex();
    if (lock == NULL) {
        return ESP_ERR_NO_MEM;
    }
    memcpy(storage->namespace_name, namespace_name, length + 1U);
    storage->lock = lock;
    storage->initialized = true;
    return ESP_OK;
}

esp_err_t dajilog_setting_storage_nvs_load(
    dajilog_setting_storage_nvs_t *storage,
    const char *key,
    uint32_t expected_schema_version,
    void *payload,
    size_t payload_size)
{
    if (!storage_ready(storage)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!key_valid(key) || payload == NULL || payload_size == 0U ||
        payload_size > UINT32_MAX) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake((SemaphoreHandle_t)storage->lock, portMAX_DELAY);
    nvs_handle_t handle = 0;
    esp_err_t error = nvs_open(storage->namespace_name, NVS_READONLY, &handle);
    size_t record_size = 0U;
    if (error == ESP_OK) {
        error = nvs_get_blob(handle, key, NULL, &record_size);
    }
    const size_t expected_size = sizeof(storage_record_header_t) + payload_size;
    if (error == ESP_OK && record_size != expected_size) {
        error = ESP_ERR_INVALID_SIZE;
    }

    uint8_t *record = NULL;
    if (error == ESP_OK) {
        record = malloc(record_size);
        if (record == NULL) {
            error = ESP_ERR_NO_MEM;
        }
    }
    if (error == ESP_OK) {
        error = nvs_get_blob(handle, key, record, &record_size);
    }
    if (handle != 0) {
        nvs_close(handle);
    }

    if (error == ESP_OK) {
        const storage_record_header_t *header =
            (const storage_record_header_t *)record;
        const void *stored_payload = record + sizeof(*header);
        if (header->magic != STORAGE_RECORD_MAGIC ||
            header->payload_size != payload_size) {
            error = ESP_ERR_INVALID_SIZE;
        } else if (header->schema_version != expected_schema_version) {
            error = ESP_ERR_INVALID_VERSION;
        } else if (header->checksum !=
                   payload_checksum(stored_payload, payload_size)) {
            error = ESP_ERR_INVALID_CRC;
        } else {
            memcpy(payload, stored_payload, payload_size);
        }
    }
    free(record);
    xSemaphoreGive((SemaphoreHandle_t)storage->lock);
    return error;
}

esp_err_t dajilog_setting_storage_nvs_save(
    dajilog_setting_storage_nvs_t *storage,
    const char *key,
    uint32_t schema_version,
    const void *payload,
    size_t payload_size)
{
    if (!storage_ready(storage)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!key_valid(key) || payload == NULL || payload_size == 0U ||
        payload_size > UINT32_MAX) {
        return ESP_ERR_INVALID_ARG;
    }

    const size_t record_size = sizeof(storage_record_header_t) + payload_size;
    uint8_t *record = malloc(record_size);
    if (record == NULL) {
        return ESP_ERR_NO_MEM;
    }
    storage_record_header_t header = {
        .magic = STORAGE_RECORD_MAGIC,
        .schema_version = schema_version,
        .payload_size = (uint32_t)payload_size,
        .checksum = payload_checksum(payload, payload_size),
    };
    memcpy(record, &header, sizeof(header));
    memcpy(record + sizeof(header), payload, payload_size);

    xSemaphoreTake((SemaphoreHandle_t)storage->lock, portMAX_DELAY);
    nvs_handle_t handle = 0;
    esp_err_t error = nvs_open(storage->namespace_name, NVS_READWRITE, &handle);
    if (error == ESP_OK) {
        error = nvs_set_blob(handle, key, record, record_size);
    }
    if (error == ESP_OK) {
        error = nvs_commit(handle);
    }
    if (handle != 0) {
        nvs_close(handle);
    }
    xSemaphoreGive((SemaphoreHandle_t)storage->lock);
    free(record);
    return error;
}

esp_err_t dajilog_setting_storage_nvs_erase(
    dajilog_setting_storage_nvs_t *storage,
    const char *key)
{
    if (!storage_ready(storage)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!key_valid(key)) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake((SemaphoreHandle_t)storage->lock, portMAX_DELAY);
    nvs_handle_t handle = 0;
    esp_err_t error = nvs_open(storage->namespace_name, NVS_READWRITE, &handle);
    if (error == ESP_OK) {
        error = nvs_erase_key(handle, key);
    }
    if (error == ESP_OK) {
        error = nvs_commit(handle);
    }
    if (handle != 0) {
        nvs_close(handle);
    }
    xSemaphoreGive((SemaphoreHandle_t)storage->lock);
    return error;
}
