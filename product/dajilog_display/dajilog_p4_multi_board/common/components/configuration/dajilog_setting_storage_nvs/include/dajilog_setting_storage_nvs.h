#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DAJILOG_NVS_NAMESPACE_CAPACITY 16U

typedef struct {
    char namespace_name[DAJILOG_NVS_NAMESPACE_CAPACITY];
    void *lock;
    bool initialized;
} dajilog_setting_storage_nvs_t;

esp_err_t dajilog_setting_storage_nvs_init(
    dajilog_setting_storage_nvs_t *storage,
    const char *namespace_name);

esp_err_t dajilog_setting_storage_nvs_load(
    dajilog_setting_storage_nvs_t *storage,
    const char *key,
    uint32_t expected_schema_version,
    void *payload,
    size_t payload_size);

esp_err_t dajilog_setting_storage_nvs_save(
    dajilog_setting_storage_nvs_t *storage,
    const char *key,
    uint32_t schema_version,
    const void *payload,
    size_t payload_size);

esp_err_t dajilog_setting_storage_nvs_erase(
    dajilog_setting_storage_nvs_t *storage,
    const char *key);

#ifdef __cplusplus
}
#endif
