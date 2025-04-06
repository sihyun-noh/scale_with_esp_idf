
#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

#if 1
#include "wifi_configuration_ap.h"
#include "ssid_manager.h"
#include "wifi_station.h"
#endif
static const char* TAG = "CPP_EXAMPLE";

class MyTask {
  public:
  static void taskFunction(void* param) {
    for (;;) {
      ESP_LOGI(TAG, "Hello from C++ Task!");
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
};

extern "C" void app_main()  // C++에서도 app_main은 기본적으로 "C"로 선언
{
  // C++ 스타일로 로그 찍기
  std::cout << "C++ is running on ESP32!" << std::endl;

  // Initialize the default event loop
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  // Initialize NVS flash for WiFi configuration
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

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

  // FreeRTOS Task 생성
  xTaskCreate(&MyTask::taskFunction, "cpp_task", 2048, NULL, 5, NULL);
}
