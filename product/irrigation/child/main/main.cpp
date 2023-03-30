#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/projdefs.h"
#include "log.h"
#include "nvs_flash.h"
#include "esp_sleep.h"
#include "esp_mac.h"

#include "shell_console.h"
#include "syscfg.h"
#include "sys_config.h"
#include "sys_status.h"
#include "wifi_manager.h"
#include "actuator.h"
#include "main.h"
#include "espnow.h"
#include "esp_mac.h"
#include "battery_task.h"
#include "wifi_manager_private.h"

#include <string.h>

#define DELAY_1SEC 1000
#define DELAY_5SEC 5000
#define DELAY_10SEC 10000
#define DELAY_500MS 500

const char* TAG = "child_app";

sc_ctx_t* sc_ctx = NULL;

#ifdef __cplusplus
extern "C" {
#endif

void create_key_task(void);
void create_control_task(void);
void create_led_task(void);

extern void on_data_recv(const uint8_t* mac, const uint8_t* incomingData, int len);
extern void on_data_sent_cb(const uint8_t* macAddr, esp_now_send_status_t status);

#ifdef __cplusplus
}
#endif

static operation_mode_t s_curr_mode;
static int main_sleep_time = 60;

void set_operation_mode(operation_mode_t mode) {
  s_curr_mode = mode;
}

operation_mode_t get_operation_mode(void) {
  return s_curr_mode;
}

static void delay(uint32_t ms) {
  vTaskDelay(ms / portTICK_PERIOD_MS);
}

/**
 * @brief Handler function to stop interactive command shell
 *
 * @note This function is called in shell_command_impl.c
 */
#ifdef __cplusplus
extern "C" void stop_shell(void) {
#else
void stop_shell(void) {
#endif
  if (sc_ctx) {
    sc_stop(sc_ctx);
  }
}

int sleep_timer_wakeup(uint64_t wakeup_time_sec) {
  int ret = 0;
  LOGI(TAG, "Enabling timer wakeup, %llus\n", wakeup_time_sec);
  ret = esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);
  esp_deep_sleep_start();
  return ret;
}

static void check_model(void) {
  char model_name[10] = { 0 };
  char power_mode[10] = { 0 };

  syscfg_set(SYSCFG_I_POWERMODE, SYSCFG_N_POWERMODE, "B");
  syscfg_get(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, model_name, sizeof(model_name));
  syscfg_get(SYSCFG_I_POWERMODE, SYSCFG_N_POWERMODE, power_mode, sizeof(power_mode));

  LOGI(TAG, "model_name : %s, power_mode : %s", model_name, power_mode);
}

int system_init(void) {
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  if (ret)
    return ERR_NVS_FLASH;

  ret = wifi_user_init();
  if (ret)
    return ERR_WIFI_INIT;

  ret = wifi_espnow_mode(WIFI_OP_AP_STA);
  if (ret)
    return ERR_WIFI_INIT;

  ret = syscfg_init();
  if (ret)
    return ERR_SYSCFG_INIT;

  ret = syscfg_open();
  if (ret)
    return ERR_SYSCFG_OPEN;

  ret = sys_stat_init();
  if (ret)
    return ERR_SYS_STATUS_INIT;

  // Generate the default manufacturing data if there is no data in mfg partition.
  generate_syscfg();

  check_model();

  valve_init();

  battery_init();

  create_key_task();

  // Get a main mac address that will be used in espnow
  uint8_t mac[6] = { 0 };
#if (CONFIG_ESPNOW_WIFI_MODE == STATION)
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
#elif (CONFIG_ESPNOW_WIFI_MODE == SOFTAP)
  esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
#endif
  LOG_BUFFER_HEX(TAG, mac, sizeof(mac));
  return SYSINIT_OK;
}

void loop_task(void) {
  uint32_t delay_ms = 0;

  set_operation_mode(START_MODE);

  while (1) {
    delay_ms = DELAY_500MS;
    switch (get_operation_mode()) {
      case START_MODE: {
        LOGI(TAG, "ESPNOW_START_MODE");
        if (espnow_start(on_data_recv, on_data_sent_cb) == false) {
          LOGE(TAG, "Failed to start espnow");
          set_operation_mode(DEEP_SLEEP_MODE);
        } else {
          set_device_onboard(1);
        }
        set_operation_mode(CONTROL_MODE);
      } break;
      case CONTROL_MODE: {
        if (is_device_onboard()) {
          LOGI(TAG, "CONTROL_MODE");
          create_control_task();
          delay_ms = DELAY_1SEC;
          set_operation_mode(MONITOR_MODE);
        } else {
          set_operation_mode(DEEP_SLEEP_MODE);
        }
      } break;
      case MONITOR_MODE: {
        if (is_device_onboard()) {
          // LOGI(TAG, "MONITOR_MODE");
          delay_ms = DELAY_1SEC;
        } else {
          set_operation_mode(DEEP_SLEEP_MODE);
        }
      } break;
      case DEEP_SLEEP_MODE: {
        LOGI(TAG, "DEEP_SLEEP_MODE");
        sleep_timer_wakeup(main_sleep_time);
      } break;
    }
    delay(delay_ms);
  }
}

#ifdef __cplusplus
extern "C" void app_main(void) {
#else
void app_main(void) {
#endif
  int rc = SYSINIT_OK;

  if ((rc = system_init()) != SYSINIT_OK) {
    LOGE(TAG, "Failed to initialize device, error = [%d]", rc);
    return;
  }

  // Start interactive shell command line
  sc_ctx = sc_init();
  if (sc_ctx)
    sc_start(sc_ctx);

  set_device_onboard(0);

  create_led_task();

  loop_task();
}
