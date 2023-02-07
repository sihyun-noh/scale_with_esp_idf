#include "easy_setup.h"
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
#include "sysfile.h"
#include "config.h"
#include "filelog.h"
#include "mqtt_task.h"
#include "sensor_task.h"
#include "adc.h"
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

void modbus_sensor_test(int mb_sensor);
void create_led_task(void);

extern int start_file_server(uint32_t port);

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
  if (is_fwupdate()) {
    s_curr_mode = OTA_FWUPDATE_MODE;
  }
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
  char reconnect[SYSCFG_S_RECONNECT] = { 0 };

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
  char model_name[SYSCFG_S_MODELNAME] = { 0 };
  char power_mode[SYSCFG_S_POWERMODE] = { 0 };
  char s_send_interval[SYSCFG_S_SEND_INTERVAL] = { 0 };

  syscfg_get(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, model_name, sizeof(model_name));
  syscfg_get(SYSCFG_I_POWERMODE, SYSCFG_N_POWERMODE, power_mode, sizeof(power_mode));
  syscfg_get(SYSCFG_I_SEND_INTERVAL, SYSCFG_N_SEND_INTERVAL, s_send_interval, sizeof(s_send_interval));
  send_interval = atoi(s_send_interval);

  LOGI(TAG, "model_name : %s, power_mode : %s", model_name, power_mode);
  LOGI(TAG, "send_interval : %d", send_interval);

  if (!strncmp(power_mode, "B", 1))
    set_battery_model(1);
  else
    set_battery_model(0);
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

  ret = init_sysfile(PARTITION_NAME, BASE_PATH);
  if (ret)
    return ERR_SPIFFS_INIT;

  // Generate the default manufacturing data if there is no data in mfg partition.
  generate_syscfg();

  check_model();

  battery_init();

  create_mqtt_task();

  if (!is_battery_model()) {
    ret = monitoring_init();
    if (ret)
      return ERR_MONITORING_INIT;
  }

  LOGI(TAG, "CPU0 reset reason: ");
  get_reset_reason(0);
  LOGI(TAG, "CPU1 reset reason: ");
  get_reset_reason(1);

  return SYSINIT_OK;
}

void battery_loop_task(void) {
  int rc = 0;
  uint32_t delay_ms = 0;

  set_operation_mode(SENSOR_INIT_MODE);

  while (1) {
    delay_ms = DELAY_500MS;
    switch (get_operation_mode()) {
      case SENSOR_INIT_MODE: {
        LOGI(TAG, "SENSOR_INIT_MODE");
        if ((rc = sensor_init()) != SYSINIT_OK) {
          LOGI(TAG, "Could not initialize sensor");
          set_operation_mode(DEEP_SLEEP_MODE);
        } else {
          set_operation_mode(SENSOR_READ_MODE);
        }
      } break;
      case SENSOR_READ_MODE: {
        LOGI(TAG, "SENSOR_READ_MODE");
        rc = sensor_read();
        if (rc == CHECK_OK) {
          if (read_battery_percentage() != CHECK_OK) {
            // Failed to read battery percentage, it will be set 0
            sysevent_set(ADC_BATTERY_EVENT, (char*)"0");
          }
          set_operation_mode(EASY_SETUP_MODE);
        } else if (rc == SENSOR_NOT_PUB) {
          set_operation_mode(DEEP_SLEEP_MODE);
        } else {
          rc = ERR_SENSOR_READ;
          LOGE(TAG, "sensor read, error = [%d]", rc);
          // FLOGI(TAG, "sensor read, error = [%d]", rc);
          set_operation_mode(DEEP_SLEEP_MODE);
        }
      } break;
      case EASY_SETUP_MODE: {
        LOGI(TAG, "EASY_SETUP_MODE");
        create_easy_setup_task();
        set_operation_mode(NTP_TIME_MODE);
      } break;
      case NTP_TIME_MODE: {
        if (is_device_onboard()) {
          struct tm timeinfo = { };
          tm_set_time(3600 * KR_GMT_OFFSET, 3600 * KR_DST_OFFSET, "pool.ntp.org", "time.google.com", "1.pool.ntp.org");
          if (tm_get_local_time(&timeinfo, 20000)) {
            set_operation_mode(MQTT_START_MODE);
          }
        } else {
          if (!is_running_setup_task()) {
            set_operation_mode(DEEP_SLEEP_MODE);
          }
        }
      } break;
      case MQTT_START_MODE: {
        if (is_device_onboard()) {
          LOGI(TAG, "MQTT_START_MODE");
          start_mqttc();
          set_operation_mode(SENSOR_PUB_MODE);
          delay_ms = DELAY_1SEC;
        } else {
          set_operation_mode(DEEP_SLEEP_MODE);
        }
      } break;
      case SENSOR_PUB_MODE: {
        // Sensor data should be published when device is onboarding and the ntp update is not running.
        if (is_device_onboard()) {
          LOGI(TAG, "SENSOR_PUB_MODE");
          mqtt_publish_sensor_data();
          stop_mqttc();
          set_operation_mode(DEEP_SLEEP_MODE);
          delay_ms = DELAY_1SEC;
        } else {
          set_operation_mode(DEEP_SLEEP_MODE);
        }
      } break;
      case NTP_UPDATE_MODE:
      case OTA_FWUPDATE_MODE: {
        // Do not anything while OTA FW updating
        delay_ms = DELAY_1SEC;
      } break;
      case DEEP_SLEEP_MODE: {
        LOGI(TAG, "DEEP_SLEEP_MODE");
        sleep_timer_wakeup(send_interval);
      } break;
    }
    delay(delay_ms);
  }
}

void plugged_loop_task(void) {
  int rc = 0;
  uint32_t delay_ms = 0;

  set_operation_mode(SENSOR_INIT_MODE);

  while (1) {
    delay_ms = DELAY_500MS;
    switch (get_operation_mode()) {
      case SENSOR_INIT_MODE: {
        if ((rc = sensor_init()) != SYSINIT_OK) {
          LOGI(TAG, "Could not initialize SHT3x sensor");
        } else {
          set_operation_mode(EASY_SETUP_MODE);
        }
      } break;
      case EASY_SETUP_MODE: {
        LOGI(TAG, "EASY_SETUP_MODE");
        create_easy_setup_task();
        set_operation_mode(NTP_TIME_MODE);
      } break;
      case NTP_TIME_MODE: {
        if (is_device_onboard()) {
          struct tm timeinfo = { };
          tm_set_time(3600 * KR_GMT_OFFSET, 3600 * KR_DST_OFFSET, "pool.ntp.org", "time.google.com", "1.pool.ntp.org");
          if (tm_get_local_time(&timeinfo, 20000)) {
            g_last_ntp_check_time = xTaskGetTickCount();
            set_operation_mode(MQTT_START_MODE);
          }
        } else {
          if (!is_running_setup_task()) {
            set_operation_mode(DEEP_SLEEP_MODE);
          }
        }
      } break;
      case MQTT_START_MODE: {
        if (is_device_onboard()) {
          LOGI(TAG, "MQTT_START_MODE!!!");
          start_file_server(8001);
          start_mqttc();
          set_operation_mode(SENSOR_READ_MODE);
        } else {
          set_operation_mode(DEEP_SLEEP_MODE);
        }
      } break;
      case SENSOR_READ_MODE: {
        if (is_device_onboard()) {
          LOGI(TAG, "SENSOR_READ_MODE!!!");
          rc = sensor_read();
          if (rc == CHECK_OK) {
            set_operation_mode(SENSOR_PUB_MODE);
          } else {
            rc = ERR_SENSOR_READ;
            LOGE(TAG, "sensor read, error = [%d]", rc);
            delay_ms = DELAY_5SEC;
          }
        } else {
          set_operation_mode(DEEP_SLEEP_MODE);
        }
      } break;
      case SENSOR_PUB_MODE: {
        // Sensor data should be published when device is onboarding and ntp update is not running.
        if (is_device_onboard()) {
          LOGI(TAG, "SENSOR_PUB_MODE!!!");
          send_msg_to_mqtt_task(MQTT_PUB_DATA_ID, NULL, 0);
          set_operation_mode(NTP_UPDATE_MODE);
          delay_ms = (send_interval * 1000);
        } else {
          set_operation_mode(DEEP_SLEEP_MODE);
        }
      } break;
      case NTP_UPDATE_MODE: {
        if (is_device_onboard()) {
          if (xTaskGetTickCount() >= g_last_ntp_check_time + pdMS_TO_TICKS(NTP_CHECK_TIME * 1000)) {
            LOGI(TAG, "Call set_ntp_time() !!!");
            set_ntp_time();
            g_last_ntp_check_time = xTaskGetTickCount();
            set_operation_mode(SENSOR_READ_MODE);
            delay_ms = DELAY_5SEC;
          } else {
            set_operation_mode(SENSOR_READ_MODE);
            delay_ms = DELAY_1SEC;
          }
        } else {
          set_operation_mode(DEEP_SLEEP_MODE);
        }
      } break;
      case OTA_FWUPDATE_MODE: {
        // Do not anything while OTA FW updating
        delay_ms = DELAY_1SEC;
      } break;
      case DEEP_SLEEP_MODE: {
        // On the power plug model, entering sleep mode means that the router or internet status is unavailable, so we
        // use deep sleep mode to reconnect to the Wi-Fi router.
        reconnect_count();
        stop_mqttc();
        sleep_timer_wakeup(30);
        set_operation_mode(SENSOR_INIT_MODE);
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

  if (is_battery_model()) {
    battery_loop_task();
  } else {
    plugged_loop_task();
  }
}
