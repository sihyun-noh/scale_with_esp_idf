#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register the 7B MoonWalker CAN observer and motor backend.
 *
 * Registration must occur before the product controller starts so that the
 * backend can be started after CAN initialization.
 */
esp_err_t dajilog_motor_control_register(void);

#ifdef __cplusplus
}
#endif
