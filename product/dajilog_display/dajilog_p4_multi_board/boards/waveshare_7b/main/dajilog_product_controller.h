#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register the 7B product controller implementation with System Core.
 *
 * The registered implementation owns the System/Storage tasks, command queues,
 * dashboard snapshot, product FSM entry points, and settings facade callbacks.
 * This function only registers callbacks; the controller starts later through
 * `dajilog_system_core_start()`.
 *
 * @return ESP_OK Registration succeeded.
 * @return ESP_ERR_INVALID_STATE Another implementation is already registered.
 */
esp_err_t dajilog_product_controller_register(void);

#ifdef __cplusplus
}
#endif
