#include "esp32/spiffs_impl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_vfs.h"
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
#include "msc.h"
#include "config.h"
#include "main.h"
#include "file_copy.h"
#include "battery_task.h"
#include "time_api.h"
#include "gpio_api.h"

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

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
static int op_time;
static int op_start_time;
static int op_end_time;

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
  char s_op_time[10] = { 0 };
  char mac_address[16] = { 0 };

  syscfg_get(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, model_name, sizeof(model_name));
  syscfg_get(SYSCFG_I_POWERMODE, SYSCFG_N_POWERMODE, power_mode, sizeof(power_mode));
  syscfg_get(SYSCFG_I_MACADDRESS, SYSCFG_N_MACADDRESS, mac_address, sizeof(mac_address));
  syscfg_get(SYSCFG_I_SEND_INTERVAL, SYSCFG_N_SEND_INTERVAL, s_send_interval, sizeof(s_send_interval));
  syscfg_get(SYSCFG_I_OP_TIME, SYSCFG_N_OP_TIME, s_op_time, sizeof(s_op_time));
  send_interval = atoi(s_send_interval);
  op_time = atoi(s_op_time);

  op_start_time = op_time / 100;
  op_end_time = op_time % 100;

  LOGI(TAG, "model_name : %s, power_mode : %s", model_name, power_mode);
  LOGI(TAG, "send_interval : %d", send_interval);
  LOGI(TAG, "msc_address : %s", mac_address);
  LOGI(TAG, "read interval : %d", send_interval);
  LOGI(TAG, "op_start_time : %d", op_start_time);
  LOGI(TAG, "op_end_time : %d", op_end_time);

  set_battery_model(1);
}

void sensor_gpio_init() {
  gpio_init(SENSOR_POWER_CONTROL_PORT, 2);
}

static int sensor_power_on() {
  return gpio_write(SENSOR_POWER_CONTROL_PORT, 1);
}
static int sensor_power_off() {
  return gpio_write(SENSOR_POWER_CONTROL_PORT, 0);
}

static int mkdir_datalogger() {
  if (mkdir(SEN_DATA_PATH_1, 0777) == -1 && errno != EEXIST) {
    printf("/storage/Sen1 directory create error: %s\n", strerror(errno));
    return -1;
  }
  if (mkdir(SEN_DATA_PATH_2, 0777) == -1 && errno != EEXIST) {
    printf("/storage/Sen2 directory create error: %s\n", strerror(errno));
    return -1;
  }
  if (mkdir(SEN_DATA_PATH_3, 0777) == -1 && errno != EEXIST) {
    printf("/storage/Sen3 directory create error: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}

bool operating_time() {
  struct tm time = { 0 };
  uint8_t curr_time;
  rtc_get_time(&time);
  printf("TIME: %04d-%02d-%02d-%02d-%02d-%02d\n", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour,
         time.tm_min, time.tm_sec);
  curr_time = time.tm_hour;
  if (curr_time >= op_start_time && curr_time < op_end_time) {
    return true;
  } else {
    return false;
  }
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

  rtc_time_init();

  ret = init_sysfile(PARTITION_NAME, BASE_PATH);
  if (ret)
    return ERR_SPIFFS_INIT;

  battery_init();

  sensor_gpio_init();

  create_usb_host_msc_task();

  create_led_task();

  return SYSINIT_OK;
}

void battery_loop_task(void) {
  int rc = 0;
  bool op_flag = true;
  set_operation_mode(SENSOR_INIT_MODE);

  while (1) {
    /*
    #if defined(SPIFFS_IMPL)
        const char* file_path1 = "/spiffs";
        file_list(file_path1);
    #elif defined(LITTLEFS_IMPL)
        const char* file_path1 = "/storage/Sen1";
        file_list(file_path1);
        const char* file_path2 = "/storage/Sen2";
        file_list(file_path2);
        const char* file_path3 = "/storage/Sen3";
        file_list(file_path3);
    #endif
    */
    op_flag = operating_time();
    if (op_flag) {
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
          sensor_power_on();
          // sensor warming up time
          vTaskDelay(3000 / portTICK_PERIOD_MS);
          if (read_battery_percentage() != CHECK_OK) {
            //  Failed to read battery percentage, it will be set 0
            LOGE(TAG, "not read to battery!");
            sysevent_set(ADC_BATTERY_EVENT, (char*)"0");
          }
          vTaskDelay(100 / portTICK_PERIOD_MS);

          if (is_usb_copying(USB_COPYING)) {  // while copying don't use of FLOG function.
            LOGE(TAG, "usb copying...");
          } else {
            if (sensor_read() == CHECK_OK) {
              set_operation_mode(SLEEP_MODE);
            } else {
              rc = ERR_SENSOR_READ;
              LOGE(TAG, "sensor read, error = [%d]", rc);
              set_operation_mode(SLEEP_MODE);
            }
          }
          sensor_power_off();
        } break;
        case SENSOR_PUB_MODE: {
          set_operation_mode(SLEEP_MODE);
        } break;
        case SLEEP_MODE: {
          LOGI(TAG, "SLEEP_MODE");
          vTaskDelay(send_interval * 1000 / portTICK_PERIOD_MS);
          set_operation_mode(SENSOR_READ_MODE);
        } break;
      }
      vTaskDelay(10 / portTICK_PERIOD_MS);
    } else {
      LOGI(TAG, "Not working");
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
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

#if defined(CONFIG_SPIFFS_PACKAGE)
  const char* file_path = "/spiffs/";
#elif defined(CONFIG_LITTLEFS_PACKAGE)
  const char* file_path = "/storage";
  if ((rc = mkdir_datalogger()) != SYSINIT_OK) {
    LOGE(TAG, "Failed to make directory, error = [%d]", rc);
    return;
  }
#endif

  file_list(file_path);
  //   Start interactive shell command line
  sc_ctx = sc_init();
  if (sc_ctx)
    sc_start(sc_ctx);

  battery_loop_task();
}
