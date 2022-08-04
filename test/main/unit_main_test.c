/* Example test application for testable component.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include "unity.h"

#include "easy_setup.h"
#include "nvs_flash.h"
#include "shell_console.h"
#include "syscfg.h"
#include "sysevent.h"
#include "sys_status.h"
#include "syslog.h"
#include "wifi_manager.h"
#include "esp_sleep.h"
#include "sysevent.h"
#include "event_ids.h"
#include "syscfg.h"
#include "gpio_api.h"
#include "time_api.h"
#include "monitoring.h"
#include "sysfile.h"

static void print_banner(const char* text);

const char* TAG = "unity_test_main_app";

typedef enum {
  SYSINIT_OK,
  ERR_NVS_FLASH,
  ERR_WIFI_INIT,
  ERR_SYSCFG_INIT,
  ERR_SYSCFG_OPEN,
  ERR_SYSEVENT_CREATE,
  ERR_SYS_STATUS_INIT,
  ERR_MONITORING_INIT,
  ERR_SPIFFS_INIT,
} err_sysinit_t;

int system_init(void) {
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  if (ret != ESP_OK) {
    return ERR_NVS_FLASH;
  }

  ret = wifi_user_init();
  if (ret != 0) {
    return ERR_WIFI_INIT;
  }

  ret = syscfg_init();
  if (ret != 0) {
    return ERR_SYSCFG_INIT;
  }

  ret = syscfg_open();
  if (ret != 0) {
    return ERR_SYSCFG_OPEN;
  }

  ret = sys_stat_init();
  if (ret != 0) {
    return ERR_SYS_STATUS_INIT;
  }

  ret = monitoring_init();
  if (ret != 0) {
    return ERR_MONITORING_INIT;
  }

  ret = sysevent_create();
  if (ret != 0) {
    return ERR_SYSEVENT_CREATE;
  }

  syslog_init();

  // Generate the default manufacturing data if there is no data in mfg partition.

  // ret = init_sysfile();
  // if (ret != 0) {
  //   return ERR_SPIFFS_INIT;
  // }

  //  create_log_file_server_task();

  //  generate_default_sysmfg();

  return SYSINIT_OK;
}

void app_main(void) {
  int rc = SYSINIT_OK;

  if ((rc = system_init()) != SYSINIT_OK) {
    LOGE(TAG, "Failed to initialize device, error = [%d]", rc);
    return;
  }

  print_banner("component_test");
  // UNITY_BEGIN();
  // unity_run_test_by_name("the test to wifi_init() and wifi_deinit() flow");
  // UNITY_END();

  // print_banner("Running all the registered tests");
  // UNITY_BEGIN();
  // unity_run_all_tests();
  // UNITY_END();

  print_banner("Starting interactive test menu");
  /* This function will not return, and will be busy waiting for UART input.
   * Make sure that task watchdog is disabled if you use this function.
   */
  unity_run_menu();
}

static void print_banner(const char* text) {
  printf("\n#### %s #####\n\n", text);
}
