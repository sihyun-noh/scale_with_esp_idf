#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"

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
#include "file_log.h"
#include "littlefs_impl.h"
#include "ota_update.h"
#include "sdi12_bitbang.h"

static const char* TAG = "main_app";

extern "C" {
extern void app_main_task(void);
extern void data_table_init(void);
extern void on_got_ip(void* arg, esp_event_base_t base, int32_t id, void* data);
extern esp_err_t file_info_helper();
}

extern void sdi12_task_init(void);

int do_user_cmd(int argc, char** argv) {
  printf("Hello from user command.\n");
  return 0;
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

  //  ESP_ERROR_CHECK(SDI12_Init(UART_NUM_1));

  //---------------fm_init ----------------------------------//
  fm_init(PARTITION_NAME, BASE_PATH);

  vTaskDelay(pdMS_TO_TICKS(100));

  // fm_file_list(BASE_PATH);
  ESP_ERROR_CHECK(file_info_helper());

  //---------------fm_init ----------------------------------//

#if 1
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &on_got_ip, NULL));
  app_eth_init();

#endif
#if 0
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));
  // Get the WiFi configuration
  auto& ssid_list = SsidManager::GetInstance().GetSsidList();
  if (ssid_list.empty()) {
    // Start the WiFi configuration AP
    auto& ap = WifiConfigurationAp::GetInstance();
    ap.SetSsidPrefix("ESP32");
    ap.Start();
    return;
  }

  // Otherwise, connect to the WiFi network
  WifiStation::GetInstance().Start();
#endif

  //---------------------mqtt_app------------------------//
  mqtt_app_start();

  //---------------------data_table------------------------//

  data_table_init();

  // ---------------ota-------------//

  get_sha256_of_partitions();
  //  ---------------ota-------------//

  // ---------------Auto detection-------------//
  // Step 1: Initialize the sensor detection system
  sensor_auto_detect_init();

  app_main_task();

  sdi12_task_init();

  sensor_ad_manager_t* mgr = sensor_ad_get_instance();
  // ---------------Auto detection-------------//

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
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  return;
}
