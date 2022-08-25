#include "easy_setup.h"
#include "nvs_flash.h"
#include "shell_console.h"
#include "syscfg.h"
#include "sysevent.h"
#include "sys_status.h"
#include "syslog.h"
#include "wifi_manager.h"
#include "esp_sleep.h"
#include "sysevent.h"
#include "event_ids.h"
#include "syscfg.h"
#include "gpio_api.h"
#include "time_api.h"
#include "monitoring.h"

#include <string.h>

typedef enum {
  SYSINIT_OK,
  ERR_NVS_FLASH,
  ERR_WIFI_INIT,
  ERR_SYSCFG_INIT,
  ERR_SYSCFG_OPEN,
  ERR_SYSEVENT_CREATE,
  ERR_SYS_STATUS_INIT,
  ERR_MONITORING_INIT,
} err_sysinit_t;

typedef enum {
  SYSTEM_INIT_MODE = 0,
  MODEL_CHECK_MODE,
  ACTUATOR_INIT_MODE,
  EASY_SETUP_MODE,
  TIME_ZONE_SET_MODE,
  MQTT_START_MODE,
  MONITOR_MODE,
  SLEEP_MODE,
} operation_mode_t;

static const char *TAG = "main_app";

sc_ctx_t *ctx = NULL;

extern int actuator_init(void);
extern void actuator_task(void);
static void generate_default_sysmfg(void);

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

/* The code below is only used for testing purpose until manufacturing data is applied */
static void generate_default_sysmfg(void) {
  char model_name[10] = { 0 };
  char power_mode[10] = { 0 };

  syscfg_get(MFG_DATA, "model_name", model_name, sizeof(model_name));
  if (model_name[0] == 0) {
    syscfg_set(MFG_DATA, "model_name", "GLAMT");
  }
  syscfg_get(MFG_DATA, "power_mode", power_mode, sizeof(power_mode));
  if (power_mode[0] == 0) {
    syscfg_set(MFG_DATA, "power_mode", "P");
  }
}

int system_init(void) {
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  if (ret != ESP_OK) {
    return ERR_NVS_FLASH;
  }

  ret = syscfg_init();
  if (ret != 0) {
    return ERR_SYSCFG_INIT;
  }

  ret = syscfg_open();
  if (ret != 0) {
    return ERR_SYSCFG_OPEN;
  }

  ret = sys_stat_init();
  if (ret != 0) {
    return ERR_SYS_STATUS_INIT;
  }

  ret = monitoring_init();
  if (ret != 0) {
    return ERR_MONITORING_INIT;
  }

  ret = sysevent_create();
  if (ret != 0) {
    return ERR_SYSEVENT_CREATE;
  }

  syslog_init();

  // Generate the default manufacturing data if there is no data in mfg partition.
  generate_default_sysmfg();

  return SYSINIT_OK;
}

int sleep_timer_wakeup(int wakeup_time_sec) {
  int ret = 0;
  LOGI(TAG, "Enabling timer wakeup, %ds\n", wakeup_time_sec);
  ret = esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);
  esp_deep_sleep_start();
  return ret;
}

void app_main(void) {
  int rc = SYSINIT_OK;
  char model_name[10] = { 0 };
  char power_mode[10] = { 0 };

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
          set_operation_mode(MODEL_CHECK_MODE);
        }
      } break;
      case MODEL_CHECK_MODE: {
        LOGI(TAG, "MODEL_CHECK_MODE");
        syscfg_get(MFG_DATA, "model_name", model_name, sizeof(model_name));
        syscfg_get(MFG_DATA, "power_mode", power_mode, sizeof(power_mode));
        LOGI(TAG, "model_name : %s, power_mode : %s", model_name, power_mode);
        set_operation_mode(ACTUATOR_INIT_MODE);
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
        if (is_device_configured() && is_device_onboard()) {
          LOGI(TAG, "TIME_ZONE_SET_MODE");
          // If sensor node onboarded on the network after finishing easy setup.
          // It need to sync the time zone and current time from the ntp server
          if (tm_is_timezone_set() == false) {
            tm_init_sntp();
            tm_apply_timesync();
            // TODO : Get the timezone using the region code of the manufacturing data.
            // Currently hardcoding the timezone to be KST-9
            tm_set_timezone("Asia/Seoul");
          } else {
            tm_apply_timesync();
            tm_set_timezone("Asia/Seoul");
          }
          set_operation_mode(MQTT_START_MODE);
        } else {
          // If the sensor is not connected to the farm network router, it should continuously try to connect.
          set_operation_mode(TIME_ZONE_SET_MODE);
        }
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
        } else {
          set_operation_mode(SLEEP_MODE);
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
