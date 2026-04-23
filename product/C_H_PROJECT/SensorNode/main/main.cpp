#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/gpio_types.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "driver/gpio.h"

// #include "etl/vector.h"
#include "ethernet_app.h"
#include "Wifi_Station.h"
#include "Wifi_Configuration_ap.h"
#include "sdkconfig.h"
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

static const char* TAG = "[main_app]";

extern "C" {
extern void strategy_task_mgr_mqtt_start(void);
extern void data_table_init(void);
extern void on_got_ip(void* arg, esp_event_base_t base, int32_t id, void* data);
extern esp_err_t file_info_helper();
extern void strategy_task_mgr_sensor_start(void);
extern void strategy_trigger_task_start(void);
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

static void start_lan() {
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &on_got_ip, NULL));
  app_eth_init();
}

static void start_wifi() {
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));
  auto& ssid_list = SsidManager::GetInstance().GetSsidList();
  if (ssid_list.empty()) {
    auto& ap = WifiConfigurationAp::GetInstance();
    ap.SetSsidPrefix("C&H-DataLogger");
    ap.Start();
    return;
  }
  WifiStation::GetInstance().Start();
}

static void blink_status_blink(led_color_t color, uint8_t times) {
  const TickType_t period = pdMS_TO_TICKS(500);

  blink_status_clear_leds(color);

  for (uint8_t i = 0; i < times; i++) {
    // ON
    blink_status_set_leds(color);
    vTaskDelay(period);

    // OFF
    blink_status_clear_leds(color);
    vTaskDelay(period);
  }
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

  // Register all the plugin commands added to this example
  ESP_ERROR_CHECK(console_cmd_all_register());

  // start console REPL
  ESP_ERROR_CHECK(console_cmd_start());

  // ESP_LOG_WARN: The minimum log level that will be shown. Messages with levels below WARN (e.g., INFO or DEBUG) will
  // be suppressed.
  esp_log_level_set("gpio", ESP_LOG_WARN);

  //------------------------------------------------------
  // File Manager (LittleFS / SPIFFS)
  //------------------------------------------------------
  fm_init(PARTITION_NAME, BASE_PATH);
  vTaskDelay(pdMS_TO_TICKS(100));
  ESP_ERROR_CHECK(file_info_helper());

  //------------------------------------------------------
  // OTA Partition SHA256 정보 획득
  //------------------------------------------------------
  get_sha256_of_partitions();

  //------------------------------------------------------
  // GPIO / LED
  //------------------------------------------------------
  ESP_ERROR_CHECK(bsp_gpio_init());
  blink_status_leds();

  //------------------------------------------------------
  // Network Initialization (Ethernet or Wi-Fi)
  //------------------------------------------------------

  int lan_sel = gpio_get_level((gpio_num_t)CONFIG_LAN_SELECT_PIN);
  int wifi_sel = gpio_get_level((gpio_num_t)CONFIG_WIFI_SELECT_PIN);

  ESP_LOGI(TAG, "LAN select: %d, WIFI select: %d", lan_sel, wifi_sel);

  if (lan_sel && !wifi_sel) {
    // LAN only
    start_lan();
    cfg_set_network_type("Ethernet(LAN)");
    blink_status_blink(LED_BLUE, 2);
  } else if (!lan_sel && wifi_sel) {
    // Wi-Fi only
    start_wifi();
    cfg_set_network_type("Wifi");
    blink_status_blink(LED_RED, 2);
  } else if (lan_sel && wifi_sel) {
    ESP_LOGW(TAG, "Both LAN and Wi-Fi selected, defaulting to LAN.");
    start_lan();
    cfg_set_network_type("Ethernet(LAN)");
    blink_status_blink(LED_GREEN, 2);
    blink_status_blink(LED_BLUE, 2);
  } else {
    ESP_LOGW(TAG, "No network selected, defaulting to Wi-Fi.");
    start_wifi();
    cfg_set_network_type("Wifi");
    blink_status_blink(LED_GREEN, 2);
    blink_status_blink(LED_RED, 2);
  }

  // #if CONFIG_ENABLE_ETHERNET_INTERFACE
  //   // Ethernet 사용
  //   ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &on_got_ip, NULL));
  //   app_eth_init();
  // #else  // Wi-Fi + SSID 매니저 기반 연결
  //   ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));
  //   auto& ssid_list = SsidManager::GetInstance().GetSsidList();
  //   if (ssid_list.empty()) {
  //     auto& ap = WifiConfigurationAp::GetInstance();
  //     ap.SetSsidPrefix("C&H-DataLogger");
  //     ap.Start();
  //     return;
  //   }
  //   WifiStation::GetInstance().Start();
  // #endif

  //------------------------------------------------------
  // Sensor Config Manager & Auto-Detect Init
  //------------------------------------------------------
  sensor_cfg_manager_t* cfg_mgr = sensor_cfg_get_instance();
  sensor_auto_detect_init();

  //------------------------------------------------------
  // Check Detect Pin Status & Update Only If Needed
  //------------------------------------------------------
  int level = 0;
  for (int portId = 0; portId < MAX_SENSOR_PORTS; portId++) {
    level = sensor_detect_pin_get_level(portId);
    vTaskDelay(pdMS_TO_TICKS(10));  // Optional debounce delay

    if (level) {  // HIGH = disconnected

      sensor_cfg_lock();

      if (cfg_mgr->cfg[portId].sensor_type != SENSOR_TYPE_UNKNOWN) {
        // Only reset if previously known sensor was registered
        cfg_mgr->cfg[portId].port = portId + 1;
        cfg_mgr->cfg[portId].sensor_type = SENSOR_TYPE_UNKNOWN;
        cfg_mgr->cfg[portId].current_state = SENSOR_PORT_STATUS_NONE;
        cfg_mgr->cfg[portId].dirty = 1;

        ESP_LOGW(TAG, "[DetectCheck] Sensor disconnected on port %d, resetting config", portId + 1);

        sensor_cfg_unlock();
        sensor_port_cfg_commit();  // Only commit if we changed something
      } else {
        // Already UNKNOWN, no need to reset or save
        sensor_cfg_unlock();
        ESP_LOGI(TAG, "[DetectCheck] Port %d already in UNKNOWN state, skipping", portId + 1);
      }

    } else {
      ESP_LOGI(TAG, "[DetectCheck] Sensor connected on port %d", portId + 1);
    }
  }

//------------------------------------------------------
// MQTT Initialization
//------------------------------------------------------
#if 1
  if (mqtt_init() != ESP_OK) {
    ESP_LOGE(TAG, "MQTT initialization failed. System halt.");
    // 예외 처리 필요 시 return 또는 오류 상태 저장
  }

  mqtt_client_ctx_t* mqtt_ctx = mqtt_handler_get_instance();
  EventBits_t bits = xEventGroupWaitBits(mqtt_ctx->mqtt_event_bit, MQTT_CONNECTED_BIT,
                                         pdTRUE,                 // clear on exit
                                         pdFALSE,                // wait any
                                         pdMS_TO_TICKS(30000));  // 30초 대기

  if (bits & MQTT_CONNECTED_BIT) {
    ESP_LOGI(TAG, "[MAIN INIT] MQTT broker connected.");
  } else {
    ESP_LOGW(TAG, "[MAIN INIT] MQTT broker connection timed out.");
  }

  //------------------------------------------------------
  // MQTT 처리 초기화
  //------------------------------------------------------
  strategy_task_mgr_mqtt_start();

  //------------------------------------------------------
  // Sensor 처리작업 초기화 (포트 기반)
  //------------------------------------------------------
  data_table_init();
  strategy_task_mgr_sensor_start();
  strategy_trigger_task_start();

  //------------------------------------------------------
  // Auto-Detect Task 시작
  //------------------------------------------------------

  strategy_autodetect_task_start();
  sensor_ad_manager_t* mgr = sensor_ad_get_instance();

#else
  //------------------------------------------------------
  // (테스트용) SDI-12 통신 태스크
  //------------------------------------------------------
  sdi12_task_init();
#endif

  //------------------------------------------------------
  // Main loop
  //------------------------------------------------------
  while (1) {
    // print_heap_summary();  // 디버깅 용
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
