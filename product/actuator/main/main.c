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
#include "main.h"

#include <string.h>

static const char *TAG = "main_app";

sc_ctx_t *ctx = NULL;

extern int actuator_init(void);
extern void actuator_task(void);
static void generate_default_sysmfg(void);
static void check_model(void);

extern int start_mqttc(void);
extern void stop_mqttc(void);
extern void mqtt_publish_actuator_data(void);

static operation_mode_t s_curr_mode;

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
void stop_shell(void) {
  if (ctx) {
    sc_stop(ctx);
  }
}

int sleep_timer_wakeup(int wakeup_time_sec) {
  int ret = 0;
  LOGI(TAG, "Enabling timer wakeup, %ds\n", wakeup_time_sec);
  ret = esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);
  esp_deep_sleep_start();
  return ret;
}

/* The code below is only used for testing purpose until manufacturing data is applied */
static void generate_default_sysmfg(void) {
  char model_name[10] = { 0 };
  char power_mode[10] = { 0 };

  syscfg_get(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, model_name, sizeof(model_name));
  if (model_name[0] == 0) {
#if (ACTUATOR_TYPE == SWITCH)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLASW");
#elif (ACTUATOR_TYPE == MOTOR)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLAMT");
#endif
  }
  syscfg_get(SYSCFG_I_POWERMODE, SYSCFG_N_POWERMODE, power_mode, sizeof(power_mode));
  if (power_mode[0] == 0) {
    syscfg_set(SYSCFG_I_POWERMODE, SYSCFG_N_POWERMODE, "P");
  }
}

static void check_model(void) {
  char model_name[10] = { 0 };
  char power_mode[10] = { 0 };

  syscfg_get(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, model_name, sizeof(model_name));
  syscfg_get(SYSCFG_I_POWERMODE, SYSCFG_N_POWERMODE, power_mode, sizeof(power_mode));

  LOGI(TAG, "model_name : %s, power_mode : %s", model_name, power_mode);

  if (!strncmp(power_mode, "B", 1))
    set_battery_model(1);
  else
    set_battery_model(0);
}

static int set_time_zone(void) {
  if (is_device_configured() && is_device_onboard()) {
    LOGI(TAG, "TIME_ZONE_SET_MODE");
    if (tm_is_timezone_set() == false)
      tm_init_sntp();

    tm_apply_timesync();
    tm_set_timezone("Asia/Seoul");
    return 0;
  }
  return 1;
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
  generate_default_sysmfg();

  check_model();

  ret = monitoring_init();
  if (ret)
    return ERR_MONITORING_INIT;

  return SYSINIT_OK;
}

void app_main(void) {
  int rc = SYSINIT_OK;

  set_operation_mode(SYSTEM_INIT_MODE);

  while (1) {
    switch (get_operation_mode()) {
      case SYSTEM_INIT_MODE: {
        LOGI(TAG, "SYSTEM_INIT_MODE");
        if ((rc = system_init()) != SYSINIT_OK) {
          LOGE(TAG, "Failed to initialize device, error = [%d]", rc);
          return;
        } else {
          // Start interactive shell command line
          ctx = sc_init();
          if (ctx) {
            sc_start(ctx);
          }
          set_device_onboard(0);
          set_operation_mode(ACTUATOR_INIT_MODE);
        }
      } break;
      case ACTUATOR_INIT_MODE: {
        LOGI(TAG, "ACTUATOR_INIT_MODE");
        if ((rc = actuator_init()) != SYSINIT_OK) {
          LOGI(TAG, "Could not initialize actuator");
          set_operation_mode(SLEEP_MODE);
        } else {
          set_operation_mode(EASY_SETUP_MODE);
        }
      } break;
      case EASY_SETUP_MODE: {
        LOGI(TAG, "EASY_SETUP_MODE");
        create_ethernet_easy_setup_task();
        set_operation_mode(TIME_ZONE_SET_MODE);
      } break;
      case TIME_ZONE_SET_MODE: {
        if (!set_time_zone())
          set_operation_mode(MQTT_START_MODE);
      } break;
      case MQTT_START_MODE: {
        if (is_device_onboard()) {
          start_mqttc();
          mqtt_publish_actuator_data();
          set_operation_mode(MONITOR_MODE);
        } else {
          set_operation_mode(SLEEP_MODE);
        }
      } break;
      case MONITOR_MODE: {
        if (is_device_onboard()) {
          actuator_task();
        }
      } break;
      case SLEEP_MODE: {
        stop_mqttc();
        vTaskDelay(5000 / portTICK_RATE_MS);
        sleep_timer_wakeup(30);
        set_operation_mode(SYSTEM_INIT_MODE);
      } break;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
