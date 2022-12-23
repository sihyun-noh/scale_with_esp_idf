#include "easy_setup.h"
#include "freertos/projdefs.h"
#include "nvs_flash.h"
#include "shell_console.h"
#include "syscfg.h"
#include "sys_config.h"
#include "sysevent.h"
#include "sys_status.h"
#include "syslog.h"
#include "wifi_manager.h"
#include "esp_sleep.h"
#include "event_ids.h"
#include "time_api.h"
#include "monitoring.h"
#include "config.h"
#include "filelog.h"
#include "adc.h"
#include "actuator.h"
#include "main.h"
#include "esp32/rom/rtc.h"

#include <string.h>
#include <time.h>

#define DELAY_1SEC 1000
#define DELAY_5SEC 5000
#define DELAY_10SEC 10000
#define DELAY_500MS 500

#define NTP_CHECK_TIME 28800  // 8 hour

const char* TAG = "main_app";

sc_ctx_t* sc_ctx = NULL;

static TickType_t g_last_ntp_check_time = 0;

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_LED_FEATURE)
void create_led_task(void);
#endif

#ifdef __cplusplus
}
#endif

static void check_model(void);

static operation_mode_t s_curr_mode;
static int send_interval;

void get_reset_reason(int icore) {
  RESET_REASON rc = NO_MEAN;
  char reset_reason[5] = { 0 };

  rc = rtc_get_reset_reason(icore);
  snprintf(reset_reason, sizeof(reset_reason), "%d", (int)rc);
  if (icore == 0) {
    syscfg_set(SYSCFG_I_CPU0_RESET_REASON, SYSCFG_N_CPU0_RESET_REASON, reset_reason);
  } else if (icore == 1) {
    syscfg_set(SYSCFG_I_CPU1_RESET_REASON, SYSCFG_N_CPU1_RESET_REASON, reset_reason);
  }

  switch (rc) {
    case POWERON_RESET:
      LOGI(TAG, "POWERON_RESET"); /**<1, Vbat power on reset*/
      break;
    case SW_RESET:
      LOGI(TAG, "SW_RESET"); /**<3, Software reset digital core*/
      break;
    case OWDT_RESET:
      LOGI(TAG, "OWDT_RESET"); /**<4, Legacy watch dog reset digital core*/
      break;
    case DEEPSLEEP_RESET:
      LOGI(TAG, "DEEPSLEEP_RESET"); /**<5, Deep Sleep reset digital core*/
      break;
    case SDIO_RESET:
      LOGI(TAG, "SDIO_RESET"); /**<6, Reset by SLC module, reset digital core*/
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
    case TGWDT_CPU_RESET:
      LOGI(TAG, "TGWDT_CPU_RESET"); /**<11, Time Group reset CPU*/
      break;
    case SW_CPU_RESET:
      LOGI(TAG, "SW_CPU_RESET"); /**<12, Software reset CPU*/
      break;
    case RTCWDT_CPU_RESET:
      LOGI(TAG, "RTCWDT_CPU_RESET"); /**<13, RTC Watch dog Reset CPU*/
      break;
    case EXT_CPU_RESET:
      LOGI(TAG, "EXT_CPU_RESET"); /**<14, for APP CPU, reseted by PRO CPU*/
      break;
    case RTCWDT_BROWN_OUT_RESET:
      LOGI(TAG, "RTCWDT_BROWN_OUT_RESET"); /**<15, Reset when the vdd voltage is not stable*/
      break;
    case RTCWDT_RTC_RESET:
      LOGI(TAG, "RTCWDT_RTC_RESET"); /**<16, RTC Watch dog reset digital core and rtc module*/
      break;
    default: LOGI(TAG, "NO_MEAN"); break;
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
#ifdef __cplusplus
extern "C" void stop_shell(void) {
#else
void stop_shell(void) {
#endif
  if (sc_ctx) {
    sc_stop(sc_ctx);
  }
}

int sleep_timer_wakeup(int wakeup_time_sec) {
  int ret = 0;
  LOGI(TAG, "Enabling timer wakeup, %ds\n", wakeup_time_sec);
  ret = esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);
  esp_deep_sleep_start();
  return ret;
}

static void reconnect_count(void) {
  int reconnect_cnt = 0;
  char reconnect[20] = { 0 };

  syscfg_get(SYSCFG_I_RECONNECT, SYSCFG_N_RECONNECT, reconnect, sizeof(reconnect));
  if (reconnect[0]) {
    reconnect_cnt = atoi(reconnect);
  }
  reconnect_cnt++;
  LOGI(TAG, "reconnect count = %d", reconnect_cnt);
  memset(reconnect, 0, sizeof(reconnect));
  snprintf(reconnect, sizeof(reconnect), "%d", reconnect_cnt);
  syscfg_set(SYSCFG_I_RECONNECT, SYSCFG_N_RECONNECT, reconnect);
}

static void delay(uint32_t ms) {
  vTaskDelay(ms / portTICK_PERIOD_MS);
}

static void check_model(void) {
  char model_name[10] = { 0 };
  char s_send_interval[10] = { 0 };

  syscfg_get(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, model_name, sizeof(model_name));
  syscfg_get(SYSCFG_I_SEND_INTERVAL, SYSCFG_N_SEND_INTERVAL, s_send_interval, sizeof(s_send_interval));
  send_interval = atoi(s_send_interval);

  LOGI(TAG, "model_name : %s, power_mode : %s", model_name, "B");
  LOGI(TAG, "send_interval : %d", send_interval);

  set_battery_model(1);
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

  // Generate the default manufacturing data if there is no data in mfg partition.
  generate_syscfg();

  check_model();

  ret = monitoring_init();
  if (ret)
    return ERR_MONITORING_INIT;

  valve_init();

  LOGI(TAG, "CPU0 reset reason: ");
  get_reset_reason(0);
  LOGI(TAG, "CPU1 reset reason: ");
  get_reset_reason(1);

  return SYSINIT_OK;
}

void loop_task(void) {
  uint32_t delay_ms = 0;

  set_operation_mode(EASY_SETUP_MODE);

  while (1) {
    delay_ms = DELAY_500MS;
    switch (get_operation_mode()) {
      case EASY_SETUP_MODE: {
        LOGI(TAG, "EASY_SETUP_MODE");
        create_easy_setup_task();
        set_operation_mode(ESP_NOW_START_MODE);
      } break;
      case ESP_NOW_START_MODE: {
        if (is_device_onboard()) {
          LOGI(TAG, "ESP_NOW_START_MODE");
          //start_esp_now();
          set_operation_mode(MONITOR_MODE);
          delay_ms = DELAY_1SEC;
        } else {
          set_operation_mode(DEEP_SLEEP_MODE);
        }
      } break;
      case MONITOR_MODE: {
        if (is_device_onboard()) {
          LOGI(TAG, "MONITOR_MODE");
          //esp_now_publish_actuator_data();
          //stop_esp_now();
          set_operation_mode(DEEP_SLEEP_MODE);
          delay_ms = DELAY_1SEC;
        } else {
          set_operation_mode(DEEP_SLEEP_MODE);
        }
      } break;
      case DEEP_SLEEP_MODE: {
        LOGI(TAG, "DEEP_SLEEP_MODE");
        sleep_timer_wakeup(send_interval);
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

#if defined(CONFIG_LED_FEATURE)
  create_led_task();
#endif

  loop_task();
}
