
#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start OTA update process
 *
 * @return ESP_OK if OTA succeeds, otherwise an error code
 */

void ota_start(void);
void get_sha256_of_partitions();
#ifdef __cplusplus
}
#endif

#endif  // OTA_UPDATE_H
