#include "freertos/projdefs.h"
#include "nvs_flash.h"
#include "shell_console.h"
#include "syscfg.h"
#include "sys_config.h"
#include "sysevent.h"
#include "sys_status.h"
#include "syslog.h"
#include "esp_sleep.h"
#include "event_ids.h"
#include "sysfile.h"
#include "config.h"
#include "main.h"

#include <string.h>

const char* TAG = "main_app";

sc_ctx_t* ctx = NULL;

extern void modbus_sensor_test(int mb_sensor);

extern int sensor_init(void);
extern int sensor_read(void);
extern int read_battery_percentage(void);
extern void sdcard_init(void);
extern void sdcard_write_data(void);

static void check_model(void);

static operation_mode_t s_curr_mode;
static int send_interval;

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

static void check_model(void) {
  char model_name[10] = { 0 };
  char power_mode[10] = { 0 };
  char s_send_interval[10] = { 0 };

  syscfg_get(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, model_name, sizeof(model_name));
  syscfg_get(SYSCFG_I_POWERMODE, SYSCFG_N_POWERMODE, power_mode, sizeof(power_mode));
  syscfg_get(SYSCFG_I_SEND_INTERVAL, SYSCFG_N_SEND_INTERVAL, s_send_interval, sizeof(s_send_interval));
  send_interval = atoi(s_send_interval);

  LOGI(TAG, "model_name : %s, power_mode : %s", model_name, power_mode);
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

  return SYSINIT_OK;
}

void battery_loop_task(void) {
  int rc = 0;

  set_operation_mode(SENSOR_INIT_MODE);

  while (1) {
    switch (get_operation_mode()) {
      case SENSOR_INIT_MODE: {
        LOGI(TAG, "SENSOR_INIT_MODE");
        if ((rc = sensor_init()) != SYSINIT_OK) {
          LOGI(TAG, "Could not initialize sensor");
          set_operation_mode(SLEEP_MODE);
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
            sysevent_set(ADC_BATTERY_EVENT, "0");
          }
          set_operation_mode(SENSOR_PUB_MODE);
        } else {
          rc = ERR_SENSOR_READ;
          LOGE(TAG, "sensor read, error = [%d]", rc);
          set_operation_mode(SLEEP_MODE);
        }
      } break;
      case SENSOR_PUB_MODE: {
        sdcard_write_data();
        set_operation_mode(SLEEP_MODE);
      } break;
      case SLEEP_MODE: {
        LOGI(TAG, "SLEEP_MODE");
        sleep_timer_wakeup(send_interval);
      } break;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void app_main(void) {
  int rc = SYSINIT_OK;

  if ((rc = system_init()) != SYSINIT_OK) {
    LOGE(TAG, "Failed to initialize device, error = [%d]", rc);
    return;
  }

  // Start interactive shell command line
  ctx = sc_init();
  if (ctx)
    sc_start(ctx);

  battery_loop_task();
}
