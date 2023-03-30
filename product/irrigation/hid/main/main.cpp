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
#include "esp32s3/rom/rtc.h"

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

void get_reset_reason(int cpu_no) {
  RESET_REASON rc = NO_MEAN;
  char reset_reason[5] = { 0 };

  rc = rtc_get_reset_reason(cpu_no);
  snprintf(reset_reason, sizeof(reset_reason), "%d", (int)rc);
  if (cpu_no == 0) {
    syscfg_set(SYSCFG_I_CPU0_RESET_REASON, SYSCFG_N_CPU0_RESET_REASON, reset_reason);
  } else if (cpu_no == 1) {
    syscfg_set(SYSCFG_I_CPU1_RESET_REASON, SYSCFG_N_CPU1_RESET_REASON, reset_reason);
  }

  switch (rc) {
    case POWERON_RESET:
      LOGI(TAG, "POWERON_RESET"); /**<1, Vbat power on reset*/
      break;
    case RTC_SW_SYS_RESET:
      LOGI(TAG, "RTC_SW_SYS_RESET"); /**<3, Software reset digital core*/
      break;
    case DEEPSLEEP_RESET:
      LOGI(TAG, "DEEPSLEEP_RESET"); /**<5, Deep Sleep reset digital core*/
      break;
    case TG0WDT_SYS_RESET:
      LOGI(TAG, "TG0WDT_SYS_RESET"); /**<7, Timer Group0 Watch dog reset digital core*/
      break;
    case TG1WDT_SYS_RESET:
      LOGI(TAG, "TG1WDT_SYS_RESET"); /**<8, Timer Group1 Watch dog reset digital core*/
      break;
    case RTCWDT_SYS_RESET:
      LOGI(TAG, "RTCWDT_SYS_RESET"); /**<9, RTC Watch dog Reset digital core*/
      break;
    case INTRUSION_RESET:
      LOGI(TAG, "INTRUSION_RESET"); /**<10, Instrusion tested to reset CPU*/
      break;
    case TG0WDT_CPU_RESET:
      LOGI(TAG, "TG0WDT_CPU_RESET"); /**<11, Time Group0 reset CPU*/
      break;
    case RTC_SW_CPU_RESET:
      LOGI(TAG, "RTC_SW_CPU_RESET"); /**<12, Software reset CPU*/
      break;
    case RTCWDT_CPU_RESET:
      LOGI(TAG, "RTCWDT_CPU_RESET"); /**<13, RTC Watch dog Reset CPU*/
      break;
    case RTCWDT_BROWN_OUT_RESET:
      LOGI(TAG, "RTCWDT_BROWN_OUT_RESET"); /**<15, Reset when the vdd voltage is not stable*/
      break;
    case RTCWDT_RTC_RESET:
      LOGI(TAG, "EXT_CPU_RESET"); /**<16, RTC Watch dog reset digital core and rtc module*/
      break;
    case TG1WDT_CPU_RESET:
      LOGI(TAG, "TG1WDT_CPU_RESET"); /**<17, Time Group1 reset CPU*/
      break;
    case SUPER_WDT_RESET:
      LOGI(TAG, "SUPER_WDT_RESET"); /**<18, super watchdog reset digital core and rtc module*/
      break;
    case GLITCH_RTC_RESET:
      LOGI(TAG, "GLITCH_RTC_RESET"); /**<19, glitch reset digital core and rtc module*/
      break;
    case EFUSE_RESET:
      LOGI(TAG, "EFUSE_RESET"); /**<20, efuse reset digital core*/
      break;
    case USB_UART_CHIP_RESET:
      LOGI(TAG, "USB_UART_CHIP_RESET"); /**<21, usb uart reset digital core */
      break;
    case USB_JTAG_CHIP_RESET:
      LOGI(TAG, "USB_JTAG_CHIP_RESET"); /**<22, usb jtag reset digital core */
      break;
    case POWER_GLITCH_RESET:
      LOGI(TAG, "POWER_GLITCH_RESET"); /**<23, power glitch reset digital core and rtc module*/
      break;
    case NO_MEAN: LOGI(TAG, "NO_MEAN"); break;
  }
}

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
    // LOG_BUFFER_HEXDUMP(TAG, data, data_len, LOG_INFO);
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
#if (CONFIG_ESPNOW_WIFI_MODE == STATION)
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
#elif (CONFIG_ESPNOW_WIFI_MODE == SOFTAP)
  esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
#endif
  LOG_BUFFER_HEX(TAG, mac, sizeof(mac));

  LOGI(TAG, "CPU0 reset reason: ");
  get_reset_reason(0);
  LOGI(TAG, "CPU1 reset reason: ");
  get_reset_reason(1);

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
          set_operation_mode(DEEPSLEEP_MODE);
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
        set_operation_mode(DEEPSLEEP_MODE);
      } break;
      case DEEPSLEEP_MODE: {
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
