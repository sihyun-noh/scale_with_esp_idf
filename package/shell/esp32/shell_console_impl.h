#ifndef _SHELL_CONSOLE_IMPL_H_
#define _SHELL_CONSOLE_IMPL_H_

#include "esp_console.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sc_ctx sc_ctx_t;

/**
 * @brief Implement code to initialize console configuration to use interactive command shell
 *
 * @return sc_ctx_t* pointer to the shell context
 */
sc_ctx_t *sc_init_impl(void);

/**
 * @brief Implement code to start interactive command shell
 *
 * @param ctx pointer to the shell context
 * @return int 0 on success, -1 on failure
 */
int sc_start_impl(sc_ctx_t *ctx);

/**
 * @brief Implement code to stop interactive command shell
 *
 * @param ctx pointer to the shell context
 */
void sc_stop_impl(sc_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* _SHELL_CONSOLE_IMPL_H_ */
