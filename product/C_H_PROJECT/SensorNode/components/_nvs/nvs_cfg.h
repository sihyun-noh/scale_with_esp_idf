
#ifndef NVS_CFG_H
#define NVS_CFG_H

/**
 * @file nvs_cfg.h
 * @brief APIs for storing and retrieving configuration values in the 'cfg' NVS partition.
 *
 * Provides helper functions to easily write and read primitive data types and
 * blobs (e.g., structures) from the 'cfg' partition in ESP32's non-volatile storage.
 */

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Initialize 'cfg' NVS partition
esp_err_t cfg_nvs_init(void);

/// Store a 32-bit unsigned integer
esp_err_t cfg_set_u32(const char *key, uint32_t value);

/// Retrieve a 32-bit unsigned integer
esp_err_t cfg_get_u32(const char *key, uint32_t *value);

/// Store a null-terminated string
esp_err_t cfg_set_str(const char *key, const char *value);

/// Retrieve a null-terminated string (memory must be freed by caller)
esp_err_t cfg_get_str(const char *key, char **value);

/// Store a blob (e.g., struct)
esp_err_t cfg_set_blob(const char *key, const void *data, size_t size);

/// Retrieve a blob (e.g., struct)
esp_err_t cfg_get_blob(const char *key, void *data_out, size_t size);

#ifdef __cplusplus
}
#endif
#endif
