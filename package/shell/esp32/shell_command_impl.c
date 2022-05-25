/**
 * @file shell_command_impl.c
 *
 * @brief Shell command implementation that will be used by the shell console
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include "shell_command_impl.h"

#include <stdio.h>

#include "esp_system.h"
#include "nvs_flash.h"
#include "shell_console_impl_priv.h"
#include "syscfg_cmd.h"
#include "syslog.h"

extern void stop_shell(void);

extern int mqtt_start_cmd(int argc, char **argv);
extern int mqtt_subscribe_cmd(int argc, char **argv);
extern int mqtt_publish_cmd(int argc, char **argv);
extern char *uptime(void);

typedef int (*sc_cmd_func_t)(int argc, char **argv);

typedef struct {
  const char *name;
  sc_cmd_func_t func;
  const char *help;
} sc_cmd_t;

/**
 * @brief Shell command implementation that will be used by the shell console
 *
 * @note This is a sample implementation of a shell command.
 *       You can implement your own shell command by following the same
 *       rest_command : command for device reboot.
 *
 * @param argc argument count
 * @param argv argument values
 * @return int return code of the command
 */
static int reset_command(int argc, char **argv) {
  printf("Resetting...\n");
  esp_restart();
  return 0;
}

static int erase_nvs_command(int argc, char **argv) {
  printf("Erasing NVS...\n");
  nvs_handle_t nvs_handle;

  esp_err_t err = nvs_open("nvs", NVS_READWRITE, &nvs_handle);
  if (err == ESP_OK) {
    err = nvs_erase_all(nvs_handle);
    if (err == ESP_OK) {
      err = nvs_commit(nvs_handle);
    }
  }

  printf("Namespace '%s' was %s erased\n", "nvs", (err == ESP_OK) ? "" : "not");

  nvs_close(nvs_handle);
  return 0;
}

/**
 * @brief Quit shell command
 *
 * @note This command to quit the shell when not in use.
 *
 * @param argc argument count
 * @param argv argument values
 * @return int return code of the command
 */
static int quit_shell_command(int argc, char **argv) {
  printf("Quitting shell...\n");
  stop_shell();
  return 0;
}

static int syslog_show(int argc, char **argv) {
  printf("Showing syslog...\n");
  dbg_syslog();
  return 0;
}

static int syslog_pub(int argc, char **argv) {
  printf("Publishing syslog...\n");
  publish_syslog();
  return 0;
}

static int uptime_cmd(int argc, char **argv) {
  printf("%s\n", uptime());
  return 0;
}

/** @brief Assign the command structure that will be used in shell command */
static sc_cmd_t commands[] = {
  {
      .name = "reset",
      .help = "Reset the device",
      .func = reset_command,
  },
  {
      .name = "quit",
      .help = "Quit the shell",
      .func = quit_shell_command,
  },
  {
      .name = "erase_nvs",
      .help = "Erase NVS",
      .func = erase_nvs_command,
  },
  {
      .name = "syscfg_get",
      .help = "Get a value from syscfg using key",
      .func = syscfg_get_cmd,
  },
  {
      .name = "syscfg_set",
      .help = "Set a value to syscfg using key",
      .func = syscfg_set_cmd,
  },
  {
      .name = "syscfg_unset",
      .help = "Unset a value to syscfg using key",
      .func = syscfg_unset_cmd,
  },
  {
      .name = "syscfg_set_blob",
      .help = "Set a blob to syscfg using key",
      .func = syscfg_set_blob_cmd,
  },
  {
      .name = "syscfg_get_blob",
      .help = "Get a blob from syscfg using key",
      .func = syscfg_get_blob_cmd,
  },
  {
      .name = "syscfg_show",
      .help = "Show all key and value from syscfg",
      .func = syscfg_show_cmd,
  },
  {
      .name = "syscfg_info",
      .help = "Show information about syscfg",
      .func = syscfg_info_cmd,
  },
  {
      .name = "syscfg_erase",
      .help = "Erase all key and value from syscfg",
      .func = syscfg_erase_cmd,
  },
  {
      .name = "syslog_show",
      .help = "Show all log message from syslog",
      .func = syslog_show,
  },
  {
      .name = "syslog_pub",
      .help = "Publish syslog message",
      .func = syslog_pub,
  },
  {
      .name = "mqtt_start",
      .help = "Start MQTT client >> mqtt_start host port",
      .func = mqtt_start_cmd,
  },
  {
      .name = "mqtt_sub",
      .help = "Subscribe MQTT topic >> mqtt_subscribe topic qos",
      .func = mqtt_subscribe_cmd,
  },
  {
      .name = "mqtt_pub",
      .help = "Publish MQTT topic >> mqtt_publish topic payload qos",
      .func = mqtt_publish_cmd,
  },
  {
      .name = "uptime",
      .help = "Device running time",
      .func = uptime_cmd,
  },
};

/**
 * @brief Register an user defined command to the shell console.
 *
 * @param ctx      The context of the shell console.
 * @param command   The command to be registered.
 * @return int      0 on success, negative on error.
 */
static int sc_register_command(sc_ctx_t *ctx, sc_cmd_t *command) {
  esp_console_cmd_t *cmd = (esp_console_cmd_t *)malloc(sizeof(esp_console_cmd_t));
  cmd->command = command->name;
  cmd->help = command->help;
  cmd->hint = NULL;
  cmd->func = command->func;
  cmd->argtable = NULL;
  ctx->cmds[ctx->cmd_count++] = cmd;
  return esp_console_cmd_register(cmd);
}

int sc_register_commands(sc_ctx_t *ctx) {
  int i;
  int count = sizeof(commands) / sizeof(sc_cmd_t);
  for (i = 0; i < count; i++) {
    sc_register_command(ctx, &commands[i]);
  }
  return 0;
}

void sc_unregister_commands(sc_ctx_t *ctx) {
  int i;
  for (i = 0; i < ctx->cmd_count; i++) {
    free(ctx->cmds[i]);
  }
}
