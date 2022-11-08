#ifndef _SHELL_CONSOLE_H_
#define _SHELL_CONSOLE_H_

#include "esp32/shell_console_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize console configuration to use interactive command shell
 *
 * @return sc_ctx_t* context of the shell console
 */
sc_ctx_t *sc_init(void);

/**
 * @brief Start interactive command shell
 *
 * @param ctx pointer to the shell context
 * @return int 0 on success, -1 on failure
 */
int sc_start(sc_ctx_t *ctx);

/**
 * @brief Stop interactive command shell
 *
 * @param ctx pointer to the shell context
 */
void sc_stop(sc_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* _SHELL_CONSOLE_H_ */
