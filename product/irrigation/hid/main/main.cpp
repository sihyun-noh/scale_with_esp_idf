#include <string.h>

#include "nvs_flash.h"
#include "esp_sleep.h"
#include "esp_mac.h"

#include "shell_console.h"
#include "syscfg.h"
#include "sys_config.h"
#include "sys_status.h"
#include "sysevent.h"
#include "syslog.h"
#include "sdcard.h"
#include "wifi_manager.h"
#include "event_ids.h"
#include "sysfile.h"
#include "config.h"
#include "main.h"
#include "espnow.h"
#include "control_task.h"
#include "filelog.h"
#include "file_manager.h"
#include "time_api.h"
#include "command.h"
#include "gui.h"

static const char *TAG = "main_app";

sc_ctx_t *sc_ctx = NULL;

uint8_t *main_mac_addr;

uint32_t start_ms = 0;

extern "C" {
extern void sdcard_init(void);
extern void sdcard_write_data(void);
}

static void check_model(void);

static operation_mode_t s_curr_mode;
static int main_sleep_delay = 30;

void set_operation_mode(operation_mode_t mode) {
  s_curr_mode = mode;
}

operation_mode_t get_operation_mode(void) {
  return s_curr_mode;
}

/**
 * @brief Handler function to stop interactive command shell
 *
 * @note This function is called in shell_command_impl.c
 */
extern "C" void stop_shell(void) {
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

  syscfg_get(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, model_name, sizeof(model_name));
  syscfg_get(SYSCFG_I_POWERMODE, SYSCFG_N_POWERMODE, power_mode, sizeof(power_mode));

  LOGI(TAG, "model_name : %s, power_mode : %s", model_name, power_mode);

  set_battery_model(1);
}

static void recv_data_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  if (memcmp(mac_addr, main_mac_addr, MAC_ADDR_LEN) == 0) {
    LOGI(TAG, "Receive Data from Main device");
    LOG_BUFFER_HEXDUMP(TAG, data, data_len, LOG_INFO);
    send_msg_to_ctrl_task((void *)data, data_len);
  }
}

static void send_data_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    LOGI(TAG, "Delivery Success to ");
  } else {
    LOGI(TAG, "Delivery Failed to ");
  }
  LOG_BUFFER_HEX(TAG, mac_addr, MAC_ADDR_LEN);
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

  ret = wifi_espnow_mode();
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

  ret = sysevent_create();
  if (ret)
    return ERR_SYSEVENT_CREATE;

  syslog_init();

  generate_syscfg();

  check_model();

  sdcard_init();

  fm_init(PARTITION_NAME, BASE_PATH);

  start_ms = millis();

  set_file_log_number(9);

  // TODO : Temporary code for testing, the code below will be removed after implementing of espnow_add_peers()
  // Get a main mac address that will be used in espnow
  uint8_t mac[6] = { 0 };
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  LOG_BUFFER_HEX(TAG, mac, sizeof(mac));

  return SYSINIT_OK;
}

bool is_time_sync_within_uptime(uint32_t uptime_ms) {
  // if time sync is not set within 10 mins after booting up, send req time sync command to the master
  if (!is_time_sync() && is_elapsed_uptime(start_ms, uptime_ms)) {
    return false;
  }
  return true;
}

void loop_task(void) {
  set_operation_mode(INIT_MODE);

  while (1) {
    switch (get_operation_mode()) {
      case INIT_MODE: {
        LOGI(TAG, "INIT_MODE");
        if (espnow_start(recv_data_cb, send_data_cb) == false) {
          LOGE(TAG, "Failed to start espnow!!!");
          set_operation_mode(SLEEP_MODE);
        } else {
          set_operation_mode(CONNECT_MODE);
        }
      } break;
      case CONNECT_MODE: {
        LOGI(TAG, "CONNECT_MODE");
        if (espnow_add_peers(HID_DEVICE)) {
          set_device_onboard(1);
          LOGI(TAG, "Success to add main addr to peer list");
        } else {
          LOGI(TAG, "Failed to add main addr to peer list");
        }
        main_mac_addr = espnow_get_master_addr();
        LOG_BUFFER_HEX(TAG, main_mac_addr, MAC_ADDR_LEN);
        set_operation_mode(MONITOR_MODE);
      } break;
      case MONITOR_MODE: {
        if (!is_time_sync_within_uptime(600 * 1000)) {
          send_command_data(REQ_TIME_SYNC, NONE, NULL, 0);
          LOGI(TAG, "Send Request Time sync command!!!");
        }
        set_operation_mode(SLEEP_MODE);
      } break;
      case SLEEP_MODE: {
        vTaskDelay(main_sleep_delay * 1000 / portTICK_PERIOD_MS);
        set_operation_mode(MONITOR_MODE);
      } break;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

extern "C" {
void app_main(void) {
  int rc = SYSINIT_OK;

  if ((rc = system_init()) != SYSINIT_OK) {
    LOGE(TAG, "Failed to initialize device, error = [%d]", rc);
    return;
  }

  // Start interactive shell command line
  sc_ctx = sc_init();
  if (sc_ctx) {
    sc_start(sc_ctx);
  }

  if (lv_display_init() != 0) {
    LOGE(TAG, "LVGL setup failed!!!");
  }

  create_control_task();

  loop_task();
}
}
