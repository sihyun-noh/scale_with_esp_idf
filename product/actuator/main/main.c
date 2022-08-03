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
  CHECK_OK,
  ERR_SENSOR_READ,
  ERR_BATTERY_READ,
} err_system_t;

static const char *TAG = "main_app";

sc_ctx_t *ctx = NULL;

extern void create_mqtt_task(void);
extern int actuator_init(void);
extern void create_actuator_task(void);
static void generate_default_sysmfg(void);

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
    syscfg_set(MFG_DATA, "model_name", "GLASW");
  }
  syscfg_get(MFG_DATA, "power_mode", power_mode, sizeof(power_mode));
  if (power_mode[0] == 0) {
    syscfg_set(MFG_DATA, "power_mode", "P");
  }

  syscfg_set(CFG_DATA, "farmip", "192.168.50.104:1883");
  // syscfg_set(CFG_DATA, "farmip", "192.168.200.191:1883");
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

typedef enum {
  SYSTEM_INIT_MODE = 0,
  MODEL_CHECK_MODE,
  ACTUATOR_INIT_MODE,
  ACTUATOR_TASK_MODE,
  EASY_SETUP_MODE,
  TIME_ZONE_SET_MODE,
  MQTT_TASK_MODE,
  MONITOR_MODE,
} operation_mode_t;

void app_main(void) {
  int rc = SYSINIT_OK;
  operation_mode_t curr_mode = SYSTEM_INIT_MODE;
  char model_name[10] = { 0 };
  char power_mode[10] = { 0 };
  char s_easy_setup[10] = { 0 };

  while (1) {
    switch (curr_mode) {
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
          curr_mode = MODEL_CHECK_MODE;
        }
      } break;
      case MODEL_CHECK_MODE: {
        syscfg_get(MFG_DATA, "model_name", model_name, sizeof(model_name));
        syscfg_get(MFG_DATA, "power_mode", power_mode, sizeof(power_mode));
        LOGI(TAG, "MODEL_CHECK_MODE");
        LOGI(TAG, "model_name : %s, power_mode : %s", model_name, power_mode);
        curr_mode = ACTUATOR_INIT_MODE;
      } break;
      case ACTUATOR_INIT_MODE: {
        LOGI(TAG, "ACTUATOR_INIT_MODE");
        if ((rc = actuator_init()) != SYSINIT_OK) {
          LOGI(TAG, "Could not initialize actuator");
          curr_mode = MONITOR_MODE;
        } else {
          curr_mode = EASY_SETUP_MODE;
        }
      } break;
      case EASY_SETUP_MODE: {
        LOGI(TAG, "EASY_SETUP_MODE");
        create_ethernet_easy_setup_task();
        curr_mode = TIME_ZONE_SET_MODE;
      } break;
      case TIME_ZONE_SET_MODE: {
        LOGI(TAG, "TIME_ZONE_SET_MODE");
        if (sysevent_get("SYSEVENT_BASE", EASY_SETUP_DONE, &s_easy_setup, sizeof(s_easy_setup)) == 0) {
          if (!strncmp(s_easy_setup, "OK", 2)) {
            if (tm_is_timezone_set() == false) {
              tm_init_sntp();
              tm_apply_timesync();
              // TODO : Get the timezone using the region code of the manufacturing data.
              // Currently hardcoding the timezone to be KST-9
              tm_set_timezone("Asia/Seoul");
            }
            curr_mode = ACTUATOR_TASK_MODE;
          }
        }
      } break;
      case ACTUATOR_TASK_MODE: {
        LOGI(TAG, "ACTUATOR_TASK_MODE");
        create_actuator_task();
        curr_mode = MQTT_TASK_MODE;
      } break;
      case MQTT_TASK_MODE: {
        LOGI(TAG, "MQTT_TASK_MODE");
        create_mqtt_task();
        curr_mode = MONITOR_MODE;
      } break;
      case MONITOR_MODE: {
      } break;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
