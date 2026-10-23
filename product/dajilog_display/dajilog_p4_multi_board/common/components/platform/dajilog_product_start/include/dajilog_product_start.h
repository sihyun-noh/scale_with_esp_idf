#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the board, start System Core, and install selected apps.
 *
 * Product `app_main()` calls this after registering board-specific services.
 */
void dajilog_product_start(void);

#ifdef __cplusplus
}
#endif
