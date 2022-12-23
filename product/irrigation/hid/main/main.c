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

sc_ctx_t* sc_ctx = NULL;

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

int set_interval_cmd(int argc, char** argv) {
  int interval = 0;

  if (argc != 2) {
    printf("Usage: 1 ~ 600 (sec)  <ex:set_interval 60>\n");
    return -1;
  }
  interval = atoi(argv[1]);
  if (interval < 1)
    return -1;

  syscfg_set(SYSCFG_I_SEND_INTERVAL, SYSCFG_N_SEND_INTERVAL, argv[1]);
  send_interval = interval;

  return 0;
}

int get_interval_cmd(int argc, char** argv) {
  char s_send_interval[10] = { 0 };

  syscfg_get(SYSCFG_I_SEND_INTERVAL, SYSCFG_N_SEND_INTERVAL, s_send_interval, sizeof(s_send_interval));
  printf("INTERVAL: %d\n", atoi(s_send_interval));

  return 0;
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

void loop_task(void) {
  int rc = 0;

  set_operation_mode(HID_INIT_MODE);

  while (1) {
    switch (get_operation_mode()) {
      case HID_INIT_MODE: {
        LOGI(TAG, "HID_INIT_MODE");
          set_operation_mode(HID_READ_MODE);
      } break;
      case HID_READ_MODE: {
        LOGI(TAG, "HID_READ_MODE");
        set_operation_mode(HID_DISPLAY_MODE);
      } break;
      case HID_DISPLAY_MODE: {
        LOGI(TAG, "HID_DISPLAY_MODE");
        set_operation_mode(SLEEP_MODE);
      } break;
      case SLEEP_MODE: {
        LOGI(TAG, "SLEEP_MODE");
        vTaskDelay(send_interval * 1000 / portTICK_PERIOD_MS);
        set_operation_mode(HID_READ_MODE);
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
  sc_ctx = sc_init();
  if (sc_ctx)
    sc_start(sc_ctx);

  loop_task();
}
