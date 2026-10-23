#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register the TR07E CAN observer used by the 7B product.
 *
 * Decoded speed and online state are forwarded to System Core; the observer
 * does not update LVGL objects directly.
 */
esp_err_t dajilog_speed_sensor_register(void);

#ifdef __cplusplus
}
#endif
