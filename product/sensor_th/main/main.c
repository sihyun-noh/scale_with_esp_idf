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

const char* TAG = "main_app";

sc_ctx_t* ctx = NULL;

extern int read_temp_humidity(void);
extern int read_battery_percentage(void);
extern int temperature_comparison(float m_temperature, float temperature);
extern void create_mqtt_task(void);
extern int sensor_init(void);

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
    syscfg_set(MFG_DATA, "model_name", "GLSTH");
  }
  syscfg_get(MFG_DATA, "power_mode", power_mode, sizeof(power_mode));
  if (power_mode[0] == 0) {
    syscfg_set(MFG_DATA, "power_mode", "B");
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

  ret = wifi_user_init();
  if (ret != 0) {
    return ERR_WIFI_INIT;
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
  SENSOR_INIT_MODE,
  SENSOR_READING_MODE,
  MODEL_CHECK_MODE,
  COMPARISON_MODE,
  BATTERY_CHECK_MODE,
  ALIVE_CHECK_MODE,
  EASY_SETUP_MODE,
  TIME_ZONE_SET_MODE,
  WIFI_CHECK_MODE,
  MQTT_TASK_MODE,
  WAITING_MODE,
  SLEEP_MODE
} operation_mode_t;

RTC_DATA_ATTR float m_temperature;
RTC_DATA_ATTR float m_humidity;
RTC_DATA_ATTR uint8_t m_alive_count;
RTC_DATA_ATTR uint8_t m_timezone_set;

int alive_check_task() {
  int ret = 0;
  LOGI(TAG, "%d", m_alive_count);
  if (m_alive_count >= 20) {
    m_alive_count = 0;
    ret = 0;
  } else {
    m_alive_count = m_alive_count + 1;
    ret = 1;
  }
  return ret;
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
  operation_mode_t curr_mode = SYSTEM_INIT_MODE;
  char model_name[10] = { 0 };
  char power_mode[10] = { 0 };
  char s_easy_setup[10] = { 0 };
  float temperature;
  float humidity;
  char s_temperature[20];
  char s_humidity[20];

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
        curr_mode = SENSOR_INIT_MODE;
      } break;
      case SENSOR_INIT_MODE: {
        LOGI(TAG, "SENSOR_INIT_MODE");
        if ((rc = sensor_init()) != SYSINIT_OK) {
          LOGI(TAG, "Could not initialize SHT3x sensor");
          curr_mode = SLEEP_MODE;
        } else {
          curr_mode = SENSOR_READING_MODE;
        }
      } break;
      case SENSOR_READING_MODE: {
        LOGI(TAG, "SENSOR_READING_MODE");
        if ((rc = read_temp_humidity()) == CHECK_OK) {
          if (!strncmp(power_mode, "P", 1)) {
            curr_mode = WIFI_CHECK_MODE;
          } else if (!strncmp(power_mode, "B", 1)) {
            curr_mode = COMPARISON_MODE;
          }
        } else {
          rc = ERR_SENSOR_READ;
          LOGE(TAG, "sensor read, error = [%d]", rc);
          curr_mode = SLEEP_MODE;
        }
      } break;
      case COMPARISON_MODE: {
        LOGI(TAG, "COMPARISON_MODE");
        sysevent_get("SYSEVENT_BASE", I2C_HUMIDITY_EVENT, &s_humidity, sizeof(s_humidity));
        sysevent_get("SYSEVENT_BASE", I2C_TEMPERATURE_EVENT, &s_temperature, sizeof(s_temperature));
        humidity = atof(s_humidity);
        temperature = atof(s_temperature);
        if ((rc = temperature_comparison(m_temperature, temperature)) == CHECK_OK) {
          curr_mode = ALIVE_CHECK_MODE;
        } else {
          curr_mode = BATTERY_CHECK_MODE;
        }
      } break;
      case BATTERY_CHECK_MODE: {
        m_alive_count = 0;
        sysevent_set(I2C_TEMPERATURE_EVENT, (char*)s_temperature);
        sysevent_set(I2C_HUMIDITY_EVENT, (char*)s_humidity);
        LOGI(TAG, "BATTERY_CHECK_MODE");
        m_temperature = temperature;
        m_humidity = humidity;
        if ((rc = read_battery_percentage()) == CHECK_OK) {
          curr_mode = EASY_SETUP_MODE;
        } else {
          rc = ERR_BATTERY_READ;
          LOGE(TAG, "battery read, error = [%d]", rc);
          curr_mode = SLEEP_MODE;
        }
      } break;
      case ALIVE_CHECK_MODE: {
        LOGI(TAG, "ALIVE_CHECK_MODE");
        if ((rc = alive_check_task()) == CHECK_OK) {
          curr_mode = BATTERY_CHECK_MODE;
        } else {
          curr_mode = SLEEP_MODE;
        }
      } break;
      case EASY_SETUP_MODE: {
        LOGI(TAG, "EASY_SETUP_MODE");
        create_easy_setup_task();
        curr_mode = TIME_ZONE_SET_MODE;
      } break;
      case TIME_ZONE_SET_MODE: {
        LOGI(TAG, "TIME_ZONE_SET_MODE");
        if (sysevent_get("SYSEVENT_BASE", EASY_SETUP_DONE, &s_easy_setup, sizeof(s_easy_setup)) == 0) {
          if (!strncmp(s_easy_setup, "OK", 2)) {
            if (!m_timezone_set) {
              if (tm_is_timezone_set() == false) {
                tm_init_sntp();
                tm_apply_timesync();
                // TODO : Get the timezone using the region code of the manufacturing data.
                // Currently hardcoding the timezone to be KST-9
                tm_set_timezone("Asia/Seoul");
              }
              m_timezone_set = 1;
            } else {
              tm_set_timezone("Asia/Seoul");
            }
            sysevent_set(EASY_SETUP_DONE, "OK");
            curr_mode = MQTT_TASK_MODE;
          }
        }
      } break;
      case WIFI_CHECK_MODE: {
        LOGI(TAG, "WIFI_CHECK_MODE");
        if (sysevent_get("SYSEVENT_BASE", EASY_SETUP_DONE, &s_easy_setup, sizeof(s_easy_setup)) == 0) {
          if (!strncmp(s_easy_setup, "OK", 2)) {
            sysevent_set(EASY_SETUP_DONE, "OK");
            curr_mode = SENSOR_READING_MODE;
          } else {
            curr_mode = EASY_SETUP_MODE;
          }
        } else {
          curr_mode = EASY_SETUP_MODE;
        }
      } break;
      case MQTT_TASK_MODE: {
        LOGI(TAG, "MQTT_TASK_MODE");
        create_mqtt_task();
        curr_mode = WAITING_MODE;
      } break;
      case WAITING_MODE: {
        LOGI(TAG, "WAITING_MODE");
        if (!strncmp(power_mode, "P", 1)) {
          curr_mode = SENSOR_READING_MODE;
        } else if (!strncmp(power_mode, "B", 1)) {
          vTaskDelay(10000 / portTICK_PERIOD_MS);
          curr_mode = SLEEP_MODE;
        }
      } break;
      case SLEEP_MODE: {
        LOGI(TAG, "SLEEP_MODE");
        sleep_timer_wakeup(60);
      } break;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
