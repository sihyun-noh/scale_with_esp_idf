#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the board, start an optional product service, and install selected apps.
 *
 * Product `app_main()` calls this after registering board-specific services.
 */
void dajilog_product_start(esp_err_t (*start_product_service)(void));

#ifdef __cplusplus
}
#endif
