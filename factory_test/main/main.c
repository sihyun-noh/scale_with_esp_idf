/* Example test application for component.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>

#include "unity.h"
#include "esp_ota_ops.h"
#include "easy_setup.h"
#include "nvs_flash.h"
#include "shell_console.h"
#include "sysevent.h"
#include "sys_status.h"
#include "syslog.h"
#include "unity_internals.h"
#include "wifi_manager.h"
#include "event_ids.h"
#include "syscfg.h"
#include "gpio_api.h"
#include "time_api.h"
#include "monitoring.h"
#include "sysfile.h"
#include "led.h"
#include "battery.h"

#include "config.h"
#include "sht3x_params.h"

static void print_banner(const char* text);

const char* TAG = "factory_test_main";

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

typedef enum {
  TEST_OK = 0,
  TEST_FAIL,
} unit_test_t;

static void __attribute__((noreturn)) task_fatal_error(void) {
  LOGE(TAG, "Exiting task due to fatal error...");
  (void)vTaskDelete(NULL);

  while (1) {
    ;
  }
}

void swap_boot_partition(void) {
  esp_err_t err;
  const esp_partition_t* update_partition = NULL;

  LOGI(TAG, "Starting OTA example");

  const esp_partition_t* running = esp_ota_get_running_partition();

  LOGI(TAG, "Current partition type %d subtype %d (offset 0x%08x)", running->type, running->subtype, running->address);

  update_partition = esp_ota_get_next_update_partition(NULL);
  assert(update_partition != NULL);
  LOGI(TAG, "Swap partition subtype %d at offset 0x%x", update_partition->subtype, update_partition->address);

  err = esp_ota_set_boot_partition(update_partition);
  if (err != ESP_OK) {
    LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
    task_fatal_error();
  }
  LOGI(TAG, "Prepare to restart system!");

  return;
}

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

  led_off(LED_RED);
  led_off(LED_GREEN);
  led_off(LED_BLUE);
  if (led_init(LED_RED) != LED_OK) {
    LOGI(TAG, "Could not initialize Red LED");
  } else if (led_init(LED_GREEN) != LED_OK) {
    LOGI(TAG, "Could not initialize Green LED");
  } else if (led_init(LED_BLUE) != LED_OK) {
    LOGI(TAG, "Could not initialize Blue LED");
  } else {
    LOGI(TAG, "led_task start");
  }
  // Generate the default manufacturing data if there is no data in mfg partition.

  // ret = init_sysfile();
  // if (ret != 0) {
  //   return ERR_SPIFFS_INIT;
  // }

  //  create_log_file_server_task();

  //  generate_default_sysmfg();

  return SYSINIT_OK;
}

//-----------------------------  sensor(battery) unit test ---------------------------//

void sensor_init_read_value_test_with_battery(void) {
  int data;
  TEST_ASSERT_EQUAL(true, battery_init());
  TEST_ASSERT_EQUAL(0, battery_read_on());
  data = read_battery(BATTERY_PORT);
  LOGI(TAG, "battery value : %d", data);
  TEST_ASSERT(0 < data);
}

//-----------------------------  sensor(sht3x) unit test ---------------------------//
void sensor_init_read_value_test_with_sht3x(void) {
  sht3x_dev_t dev;
  float temperature;
  float humidity;

  TEST_ASSERT_EQUAL(0, sht3x_init(&dev, &sht3x_params[0]));
  TEST_ASSERT_EQUAL(0, sht3x_read(&dev, &temperature, &humidity));
  LOGI(TAG, "sensor read temperature value : %.2f", temperature);
  LOGI(TAG, "sensor read humidity value : %.2f", humidity);
}

//-----------------------------  syscfg unit test ---------------------------//
void system_package_unit_test_with_syscfg(void) {
  LOGI(TAG, "CFG_DATA");
  TEST_ASSERT_EQUAL(0, syscfg_show(CFG_DATA));
  LOGI(TAG, "MFG_DATA");
  TEST_ASSERT_EQUAL(0, syscfg_show(MFG_DATA));
}

//-----------------------------  wifi unit test ---------------------------//
void system_package_unit_test_with_wifi(void) {
  char res_event_msg[50] = { 0 };
  int try_count = 0;
  int ret = -1;
  unit_test_t test_status = TEST_OK;

  TEST_ASSERT_EQUAL(0, wifi_sta_mode());
  TEST_ASSERT_EQUAL(0, wifi_connect_ap("COMFAST_TEST_01", "admin123"));
  while (1) {
    ret = sysevent_get(IP_EVENT, IP_EVENT_STA_GOT_IP, &res_event_msg, sizeof(res_event_msg));
    if (!ret) {
      LOGI(TAG, "IP_EVENT_STA_GOT_IP : %s\n", res_event_msg);
      break;
    }

    if (try_count == 30) {
      test_status = TEST_FAIL;
      break;
    }
    try_count++;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  TEST_ASSERT_EQUAL(0, wifi_disconnect_ap());
  TEST_ASSERT_EQUAL(0, wifi_stop_mode());

  if (test_status == TEST_FAIL) {
    TEST_FAIL();
  }
}

void app_main(void) {
  int rc = SYSINIT_OK;
  unit_test_t test_status = TEST_OK;

  if ((rc = system_init()) != SYSINIT_OK) {
    LOGE(TAG, "Failed to initialize device, error = [%d]", rc);
    return;
  }

  print_banner("component_test_sensor_sht3x");
  UNITY_BEGIN();
  RUN_TEST(sensor_init_read_value_test_with_sht3x);
  RUN_TEST(system_package_unit_test_with_syscfg);
  RUN_TEST(system_package_unit_test_with_wifi);
  test_status = UNITY_END();

  if (test_status == TEST_OK) {
    led_on(LED_GREEN);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    swap_boot_partition();
  } else {
    led_on(LED_RED);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

static void print_banner(const char* text) {
  printf("\n#### %s #####\n\n", text);
}
