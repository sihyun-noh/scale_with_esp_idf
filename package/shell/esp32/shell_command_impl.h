#ifndef _SHELL_COMMAND_IMPL_H_
#define _SHELL_COMMAND_IMPL_H_

#include "shell_console_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Register the command list to the shell console.
 *
 * @param ctx The context of the shell console.
 * @return 0 on success, negative on error.
 */
int sc_register_commands(sc_ctx_t *ctx);

/** @brief Unregister the command list to the shell console.
 *
 * @param ctx The context of the shell console.
 */
void sc_unregister_commands(sc_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif  // _SHELL_COMMAND_IMPL_H_
