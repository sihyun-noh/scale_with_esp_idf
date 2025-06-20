#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "driver/gpio.h"

#include "etl/vector.h"
#include "ethernet_app.h"
#include "Wifi_Station.h"
#include "Wifi_Configuration_ap.h"
#include "sensor_cfg_config.h"
#include "ssid_manager.h"
#include "config.h"
#include "console_init.h"
#include "nvs_ext.h"
#include "nvs_cfg.h"
#include "mqtt_config.h"
#include "sensor_auto_detect.h"
#include "file_manager.h"
#include "littlefs_impl.h"
#include "ota_update.h"
#include "bsp_gpio.h"
#include "sdi12_task.h"

static const char* TAG = "main_app";

extern "C" {
extern void strategy_task_mgr_mqtt_start(void);
extern void data_table_init(void);
extern void on_got_ip(void* arg, esp_event_base_t base, int32_t id, void* data);
extern esp_err_t file_info_helper();
extern void strategy_task_mgr_sensor_start(void);
extern void strategy_trigger_task_start(void);
}

int do_user_cmd(int argc, char** argv) {
  printf("Hello from user command.\n");
  return 0;
}

void print_heap_summary() {
  multi_heap_info_t info;
  heap_caps_get_info(&info, MALLOC_CAP_8BIT);
  size_t internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);

  ESP_LOGI("HEAP", "Internal RAM : %u bytes", internal);
  ESP_LOGI("HEAP", "Total_free_bytes(byte)      : %u", info.total_free_bytes);
  ESP_LOGI("HEAP", "Total Alloc count(bytes)    : %u", info.total_allocated_bytes);
  ESP_LOGI("HEAP", "Minimum ever(byte)   : %u", info.minimum_free_bytes);
  ESP_LOGI("HEAP", "Largest free   : %u", info.largest_free_block);
  ESP_LOGI("HEAP", "Alloc count(block)    : %u", info.allocated_blocks);
  ESP_LOGI("HEAP", "Free count(blocks)     : %u", info.free_blocks);
  ESP_LOGI("HEAP", "total count(blocks)     : %u", info.total_blocks);
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, FW_VERSION);
  ESP_LOGI(TAG, "[APP] Startup..");
  ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
  ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

  ESP_ERROR_CHECK(esp_event_loop_create_default());

  ESP_ERROR_CHECK(nvs_init());
  ESP_ERROR_CHECK(cfg_nvs_init());
  ESP_ERROR_CHECK(cfg_init_device_id());
  ESP_ERROR_CHECK(sensor_port_cfg_init());

  // Initialize console REPL
  ESP_ERROR_CHECK(console_cmd_init());

  // Register user command
  ESP_ERROR_CHECK(console_cmd_user_register("user", do_user_cmd));

  // Register all the plugin commands added to this example
  ESP_ERROR_CHECK(console_cmd_all_register());

  // start console REPL
  ESP_ERROR_CHECK(console_cmd_start());

  //---------------fm_init ----------------------------------//
  fm_init(PARTITION_NAME, BASE_PATH);

  vTaskDelay(pdMS_TO_TICKS(100));

  // fm_file_list(BASE_PATH);
  ESP_ERROR_CHECK(file_info_helper());

  //---------------fm_init ----------------------------------//

  // ---------------ota-------------//

  get_sha256_of_partitions();
  //  ---------------ota-------------//

#if 1
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &on_got_ip, NULL));
  app_eth_init();

#else if
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));
  // Get the WiFi configuration
  auto& ssid_list = SsidManager::GetInstance().GetSsidList();
  if (ssid_list.empty()) {
    // Start the WiFi configuration AP
    auto& ap = WifiConfigurationAp::GetInstance();
    ap.SetSsidPrefix("ESP32-C&H");
    ap.Start();
    return;
  }

  // Otherwise, connect to the WiFi network
  WifiStation::GetInstance().Start();
#endif

  ESP_ERROR_CHECK(bsp_gpio_init());
  blink_status_leds();

  // cfg instance Initialize
  sensor_cfg_manager_t* cfg_mgr = sensor_cfg_get_instance();

  sensor_auto_detect_init();
//----------------------------------------------------------------//
// MQTT Publishing Strategy Initialization
//----------------------------------------------------------------//
#if 1
  mqtt_init();
  strategy_task_mgr_mqtt_start();
  //----------------------------------------------------------------//
  // Port-wise Sensing Strategy Initialization
  //----------------------------------------------------------------//

  // structure protective
  // sensor_cfg_lock();
  // // This must be set up before use.
  // cfg_mgr->cfg[5].port = 6;
  // cfg_mgr->cfg[5].current_state = SENSOR_PORT_STATUS_READY;
  // cfg_mgr->cfg[5].sensor_type = TEROS12;
  // cfg_mgr->cfg[5].columns_size = 3;
  // cfg_mgr->cfg[5].rows_size = 1;
  // cfg_mgr->cfg[5].server_config.publish_interval = 1;
  // sensor_cfg_unlock();

  // data_table Initialize
  data_table_init();
  // strategy task Initialize
  strategy_task_mgr_sensor_start();
  // strategy task trigger Initialize
  strategy_trigger_task_start();

  //----------------------------------------------------------------//
  // Auto-Detection Strategy Initialization
  //----------------------------------------------------------------//
  // Step 1: Initialize the sensor detection system
  xTaskCreate(sensor_auto_detect_task, "sensor_auto_detect_task", 4096, NULL, 5, NULL);
  sensor_ad_manager_t* mgr = sensor_ad_get_instance();
#else if

  sdi12_task_init();

#endif
  while (1) {
    // // Step 3: Wait for sensor connection events from any port (clear bits after wait)
    // EventBits_t bits = xEventGroupGetBits(mgr->sensor_event_group);
    //
    // // Step 4: Process each port if event occurred and sensor is connected
    // for (int portId = 0; portId < MAX_SENSOR_PORTS; portId++) {
    //   if (bits & SENSOR_PORT_EVENT_BIT(portId)) {
    //     if (sensor_is_connected(portId)) {
    //       ESP_LOGI(TAG, "[PORT %d] Sensor connected, start measurement...", portId);
    //
    //       ESP_ERROR_CHECK(sensor_buffer_select_port(portId));
    //       vTaskDelay(pdMS_TO_TICKS(20));
    //
    //       // TODO: Add actual measurement routine here
    //       // e.g., SDI12_StartMeasurement(...) + SDI12_SendData(...)
    //
    //       ESP_ERROR_CHECK(sensor_buffer_disable());
    //
    //     } else {
    //       ESP_LOGW(TAG, "[PORT %d] Event triggered but sensor is not connected. --> Skip", portId);
    //     }
    //   }
    // }

    // Optional: Delay between event checks

    // print_heap_summary();

    // EventBits_t bits = xEventGroupGetBits(mgr->sensor_event_group);
    //
    // if (bits & BIT0) {
    //   ESP_LOGI(TAG, "BIT0 is SET");
    // } else {
    //   ESP_LOGI(TAG, "BIT0 is CLEAR");
    // }
    //
    // if (bits & BIT1) {
    //   ESP_LOGI(TAG, "BIT1 is SET");
    // } else {
    //   ESP_LOGI(TAG, "BIT1 is CLEAR");
    // }
    // if (bits & BIT2) {
    //   ESP_LOGI(TAG, "BIT2 is SET");
    // } else {
    //   ESP_LOGI(TAG, "BIT2 is CLEAR");
    // }
    // if (bits & BIT5) {
    //   ESP_LOGI(TAG, "BIT5 is SET");
    // } else {
    //   ESP_LOGI(TAG, "BIT5 is CLEAR");
    // }
    //
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  return;
}
