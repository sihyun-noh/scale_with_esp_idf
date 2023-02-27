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
#include "sdkconfig.h"
#include "shell_command_impl.h"
#include "shell_console_impl_priv.h"
#include "syscfg_cmd.h"
#include "syslog.h"

#include <stdio.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_idf_version.h"

#include <freertos/FreeRTOS.h>

#include "icmp_echo_cmd.h"
#include "sysfile.h"
#include "wifi_manager.h"
#include "time_api.h"

extern void stop_shell(void);

#if defined(MQTTC_PACKAGE)
extern int mqtt_start_cmd(int argc, char **argv);
extern int mqtt_subscribe_cmd(int argc, char **argv);
extern int mqtt_publish_cmd(int argc, char **argv);
#endif

extern char *uptime(void);

#if (CONFIG_SENSOR_ATLAS_PH)
extern int atlas_ph_cal_cmd(int argc, char **argv);
#elif (CONFIG_SENSOR_ATLAS_EC)
extern int atlas_ec_cal_cmd(int argc, char **argv);
extern int atlas_ec_probe_cmd(int argc, char **argv);
#endif
#if (CONFIG_FILECOPY_PACKAGE)
extern int get_file_list_cmd(int argc, char **argv);
extern int get_file_read_cmd(int argc, char **argv);
#endif

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

static int sysfile_show_cmd(int argc, char **argv) {
  printf("Show sysfile list");
  sysfile_show_file();
  return 0;
}

static int free_mem(int argc, char **argv) {
  printf("Show free heap memory\n");
  int freeHeap = xPortGetFreeHeapSize();
  printf("Free HeapSize = %d\n", freeHeap);
  return 0;
}

static int scan_network(int argc, char **argv) {
  printf("Scan nearby AP network\n");

  uint16_t scan_num = 0;
  ap_info_t *ap_info_list = NULL;
  uint16_t scan_ap_num = wifi_scan_network(false, false, false, 500, 1, NULL, NULL);
  if (scan_ap_num > 0) {
    ap_info_list = get_wifi_scan_list(&scan_num);
    if (ap_info_list) {
      printf("scan ap num [%d], [%d]\n", scan_ap_num, scan_num);
      for (int i = 0; i < scan_num; i++) {
        printf("ap_info_list[%d].ssid = %s\n", i, ap_info_list[i].ssid);
        printf("ap_info_list[%d].rssi = %d\n", i, ap_info_list[i].rssi);
      }
      free(ap_info_list);
    }
  }
  return 0;
}

static int sdk_version(int argc, char **argv) {
  const char *idf_ver = esp_get_idf_version();
  printf("idf_version = %s\n", idf_ver);
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
      .func = nvs_erase_cmd,
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
      .name = "syscfg_dump",
      .help = "Show all variables with CFG and MFG",
      .func = syscfg_dump_cmd,
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
#if defined(MQTTC_PACKAGE)
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
#endif
  {
      .name = "uptime",
      .help = "Device running time",
      .func = uptime_cmd,
  },
  {
      .name = "ping",
      .help = "send ICMP ECHO_REQUEST to network hosts",
      .func = ping_cmd,
  },
  {
      .name = "show_sysfile_name",
      .help = "Show all sysfile name from ./SPIFFS ",
      .func = sysfile_show_cmd,
  },
  {
      .name = "free_mem",
      .help = "Get free heap memory",
      .func = free_mem,
  },
  {
      .name = "scan_network",
      .help = "Scan AP network",
      .func = scan_network,
  },
  {
      .name = "sdk_version",
      .help = "Get IDF-SDK version",
      .func = sdk_version,
  },
#if (CONFIG_SENSOR_ATLAS_PH)
  {
      .name = "atlas_ph_cal",
      .help = "Atlas pH Sensor Calibration",
      .func = atlas_ph_cal_cmd,
  },
#endif
#if (CONFIG_SENSOR_ATLAS_EC)
  {
      .name = "atlas_ec_cal",
      .help = "Atlas EC Sensor Calibration",
      .func = atlas_ec_cal_cmd,
  },
  {
      .name = "atlas_ec_probe",
      .help = "Atlas EC Sensor Probe",
      .func = atlas_ec_probe_cmd,
  },
#endif
  {
      .name = "rtc_set_time",
      .help = "Set RTC Time",
      .func = rtc_set_time_cmd,
  },
  {
      .name = "rtc_get_time",
      .help = "Get RTC Time",
      .func = rtc_get_time_cmd,
  },
  {
      .name = "set_interval",
      .help = "Set interval",
      .func = set_interval_cmd,
  },
  {
      .name = "get_interval",
      .help = "Get interval",
      .func = get_interval_cmd,
  },
  {
      .name = "set_op_time",
      .help = "Set operation time",
      .func = set_op_time_cmd,
  },
  {
      .name = "get_op_time",
      .help = "Get operation time",
      .func = get_op_time_cmd,
  },
#if (CONFIG_FILECOPY_PACKAGE)
  {
      .name = "get_file_list",
      .help = "Get file list",
      .func = get_file_list_cmd,
  },
  {
      .name = "get_file_read",
      .help = "Get file read",
      .func = get_file_read_cmd,
  },
#endif
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
