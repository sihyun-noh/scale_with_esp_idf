/**
 * @file shell_console_impl.c
 *
 * @brief Implement wrapper APIs to support the interactive command shell in ESP32
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include "shell_console_impl.h"
#include "shell_console_impl_priv.h"
#include "shell_command_impl.h"

#include "esp_console.h"

/*!< This is a prompt string, it depens on configuration name of target (e.g) esp32 => PROMPT_STR is "esp32>" */
#define PROMPT_STR CONFIG_IDF_TARGET
#define CONSOLE_MAX_COMMAND_LINE_LENGTH (256) /*!< Maximum length of command line */

sc_ctx_t *sc_init_impl(void) {
  esp_console_repl_t *repl = NULL;
  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();

  /* Prompt to be printed before each line.
   * This can be customized, made dynamic, etc.
   */
  repl_config.prompt = PROMPT_STR ">";
  repl_config.max_cmdline_length = CONSOLE_MAX_COMMAND_LINE_LENGTH;

  int res = esp_console_new_repl_uart(&uart_config, &repl_config, &repl);
  if (res != ESP_OK) {
    return (sc_ctx_t *)NULL;
  }

  sc_ctx_t *ctx = (sc_ctx_t *)malloc(sizeof(sc_ctx_t));
  ctx->repl = repl;
  ctx->cmd_count = 0;
  for (int i = 0; i < SC_MAX_CMDS; i++) {
    ctx->cmds[i] = NULL;
  }
  return ctx;
}

int sc_start_impl(sc_ctx_t *ctx) {
  int res = ESP_FAIL;
  if (ctx) {
    sc_register_commands(ctx);
    res = esp_console_start_repl(ctx->repl);
  }
  return res;
}

void sc_stop_impl(sc_ctx_t *ctx) {
  if (ctx) {
    sc_unregister_commands(ctx);
    ctx->repl->del(ctx->repl);
    free(ctx);
  }
}
