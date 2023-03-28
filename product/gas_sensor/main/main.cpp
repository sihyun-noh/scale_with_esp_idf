#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"
#include "shell_console.h"
#include "syscfg.h"
#include "sys_config.h"
#include "sysevent.h"
#include "sys_status.h"
#include "log.h"
#include "sysfile.h"
#include "main.h"
#include "file_copy.h"
#include "time_api.h"

#define DELAY_100MS 100
#define DELAY_1SEC 1000

const char* TAG = "main_app";

sc_ctx_t* sc_ctx = NULL;

#ifdef __cplusplus
extern "C" {
#endif
extern int sensor_init(void);
extern int sensor_read(void);
extern void create_led_task(void);
extern void create_usb_host_msc_task(void);

#ifdef __cplusplus
}
#endif

static void check_model(void);
static operation_mode_t s_curr_mode;
static int send_interval;

uint8_t mac[6] = { 0 };

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

static void delay(uint32_t ms) {
  vTaskDelay(ms / portTICK_PERIOD_MS);
}

static void check_model(void) {
  char model_name[10] = { 0 };
  char power_mode[10] = { 0 };
  char s_send_interval[10] = { 0 };
  char mac_address[16] = { 0 };

  syscfg_get(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, model_name, sizeof(model_name));
  syscfg_get(SYSCFG_I_POWERMODE, SYSCFG_N_POWERMODE, power_mode, sizeof(power_mode));
  syscfg_get(SYSCFG_I_MACADDRESS, SYSCFG_N_MACADDRESS, mac_address, sizeof(mac_address));
  syscfg_get(SYSCFG_I_SEND_INTERVAL, SYSCFG_N_SEND_INTERVAL, s_send_interval, sizeof(s_send_interval));
  send_interval = atoi(s_send_interval);

  LOGI(TAG, "model_name : %s, power_mode : %s", model_name, power_mode);
  LOGI(TAG, "mac_address : %s", mac_address);
  LOGI(TAG, "sensor read interval : %d", send_interval);

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

  generate_syscfg();

  check_model();

  ret = init_sysfile(PART_NAME, ROOT_PATH);
  if (ret)
    return ERR_FILESYSTEM_INIT;

  create_usb_host_msc_task();

  create_led_task();

  set_usb_copying(USB_COPY_FAIL, 1);

  rtc_time_init();

  set_usb_copying(USB_COPY_FAIL, 0);

  return SYSINIT_OK;
}

void loop_task(void) {
  set_operation_mode(SENSOR_INIT_MODE);

  while (1) {
    switch (get_operation_mode()) {
      case SENSOR_INIT_MODE: {
        LOGI(TAG, "SENSOR_INIT_MODE");
        if ((sensor_init()) != SYSINIT_OK) {
          LOGE(TAG, "Could not initialize sensor");
          set_operation_mode(SLEEP_MODE);
        } else {
          set_operation_mode(SENSOR_READ_MODE);
        }
      } break;
      case SENSOR_READ_MODE: {
        LOGI(TAG, "SENSOR_READ_MODE");
        if (is_usb_copying(USB_COPYING)) {
          LOGE(TAG, "usb copying...");
        } else {
          if (sensor_read() != CHECK_OK) {
            LOGE(TAG, "sensor read error");
          }
          set_operation_mode(SLEEP_MODE);
        }
      } break;
      case SLEEP_MODE: {
        LOGI(TAG, "SLEEP_MODE");
        delay(send_interval * DELAY_1SEC);
        set_operation_mode(SENSOR_READ_MODE);
      } break;
    }
    delay(DELAY_100MS);
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

  const char* file_path = DIR_PATH;
  if ((rc = make_dir(file_path)) != SYSINIT_OK) {
    LOGE(TAG, "Failed to make directory");
    return;
  }

  file_list(file_path);

  sc_ctx = sc_init();
  if (sc_ctx)
    sc_start(sc_ctx);

  loop_task();
}
