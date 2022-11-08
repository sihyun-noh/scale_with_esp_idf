/**
 * THIS "PRIVATE" HEADER SHOULD ONLY BE INCLUDED BY SOURCE FILES WHICH NEED FULL ACCESS TO THE STRUCT DEFINITION BELOW.
 * It should NOT generally be include by regular users of your API,
 * since your architectural goal is probably to have some level of data hiding to hide the contents of this struct
 * from your regular API users.
 */

/** @file shell_console_impl_priv.h
 *
 * @brief Private header for shell_console_impl.c
 *
 * @note  This header file is a private header to use when you want to hide data from the general API.
 *        Please refer to the detailed information about "opaque struct" in the site below.
 *        https://stackoverflow.com/questions/70194681/how-to-hide-the-struct-implementation-and-avoid-variable-has-incomplete-type-at/70195119#70195119
 */

#ifndef _SHELL_CONSOLE_IMPL_PRIV_H_
#define _SHELL_CONSOLE_IMPL_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SC_MAX_CMDS 10

/**
 * @brief The context of the shell console.
 *
 * @note  The context of shell console is an opaque struct. The data of this struct will be different for each platform.
 *        So we need to make a private header file to hide the data of this struct from the generic(upper) API.
 */
struct sc_ctx {
  esp_console_repl_t *repl;
  esp_console_cmd_t *cmds[SC_MAX_CMDS];
  int cmd_count;
};

#ifdef __cplusplus
}
#endif

#endif /* _SHELL_CONSOLE_IMPL_PRIV_H_ */
