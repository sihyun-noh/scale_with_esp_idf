#include <string.h>

#include "nvs_flash.h"
#include "esp_sleep.h"
#include "esp_mac.h"

#include "shell_console.h"
#include "syscfg.h"
#include "sys_config.h"
#include "sys_status.h"
#include "sysevent.h"
#include "syslog.h"
#include "sdcard.h"
#include "wifi_manager.h"
#include "event_ids.h"
#include "sysfile.h"
#include "config.h"
#include "main.h"
#include "control_task.h"
#include "filelog.h"
#include "file_manager.h"
#include "time_api.h"
#include "command.h"
#include "gui.h"
#include "esp32s3/rom/rtc.h"

#include "gpio_api.h"
#include "i2s_speaker.h"
#if CONFIG_USB_HOST_FEATURE
#include "msc.h"
#endif
#include "file_copy.h"
#include "scale_read_485.h"
#include "ui.h"

typedef enum { INPUT = 0, INPUT_PULLUP, OUTPUT } gpio_hal_mode_t;

static const char *TAG = "main_app";

sc_ctx_t *sc_ctx = NULL;

char msc_mode_check[5] = { 0 };
char indicator_model_buf[20] = { 0 };

extern "C" {
extern void sdcard_init(void);
extern void sdcard_write_data(void);
extern int sensor_init(void);
extern void create_usb_host_msc_task(void);
extern void create_usb_host_msc_ota_task(void);

// extern int read_weight(void);
}

static void check_model(void);

/**
 * @brief Handler function to stop interactive command shell
 *
 * @note This function is called in shell_command_impl.c
 */
extern "C" void stop_shell(void) {
  if (sc_ctx) {
    sc_stop(sc_ctx);
  }
}

static void check_model(void) {
  char model_name[10] = { 0 };
  char power_mode[10] = { 0 };

  syscfg_get(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, model_name, sizeof(model_name));
  syscfg_get(SYSCFG_I_POWERMODE, SYSCFG_N_POWERMODE, power_mode, sizeof(power_mode));

  LOGI(TAG, "model_name : %s, power_mode : %s", model_name, power_mode);

  set_battery_model(1);
}

void SET_CH2_PIN() {
  gpio_write(LCD_GPIO_1, 0);
  gpio_write(LCD_GPIO_2, 0);
  gpio_write(LCD_GPIO_3, 0);
}

void SET_CH1_PIN() {
  gpio_write(LCD_GPIO_1, 0);
  gpio_write(LCD_GPIO_2, 1);
  gpio_write(LCD_GPIO_3, 0);
}

void SET_MUX_CONTROL(mux_ctrl_t ch) {
  switch (ch) {
    case CH_1_SET:
      SET_CH1_PIN();
      /* code */
      break;
    case CH_2_SET:
      SET_CH2_PIN();
      /* code */
      break;
    default: break;
  }
}

void led_1_ctrl(uint8_t ctrl) {
  if (ctrl)
    gpio_write(LCD_GPIO_4, 0);
  else
    gpio_write(LCD_GPIO_4, 1);
}

void led_2_ctrl(uint8_t ctrl) {
  if (ctrl)
    gpio_write(LCD_GPIO_5, 0);
  else
    gpio_write(LCD_GPIO_5, 1);
}

void led_3_ctrl(uint8_t ctrl) {
  if (ctrl)
    gpio_write(LCD_GPIO_6, 0);
  else
    gpio_write(LCD_GPIO_6, 1);
}

int gpio_init_to_sc01_IO() {
  int ret;
  gpio_write(LCD_GPIO_1, 1);
  gpio_write(LCD_GPIO_2, 1);
  gpio_write(LCD_GPIO_3, 1);
  gpio_write(LCD_GPIO_4, 1);
  gpio_write(LCD_GPIO_5, 1);
  gpio_write(LCD_GPIO_6, 1);
  if ((ret = gpio_init(LCD_GPIO_1, OUTPUT)) != 0) {
    LOGE(TAG, "Could not initialize GPIO %d, error = %d\n", LCD_GPIO_1, ret);
    return ret;
  }
  if ((ret = gpio_init(LCD_GPIO_2, OUTPUT)) != 0) {
    LOGE(TAG, "Could not initialize GPIO %d, error = %d\n", LCD_GPIO_2, ret);
    return ret;
  }
  if ((ret = gpio_init(LCD_GPIO_3, OUTPUT)) != 0) {
    LOGE(TAG, "Could not initialize GPIO %d, error = %d\n", LCD_GPIO_3, ret);
    return ret;
  }
  if ((ret = gpio_init(LCD_GPIO_4, OUTPUT)) != 0) {
    LOGE(TAG, "Could not initialize GPIO %d, error = %d\n", LCD_GPIO_4, ret);
    return ret;
  }
  if ((ret = gpio_init(LCD_GPIO_5, OUTPUT)) != 0) {
    LOGE(TAG, "Could not initialize GPIO %d, error = %d\n", LCD_GPIO_5, ret);
    return ret;
  }
  if ((ret = gpio_init(LCD_GPIO_6, OUTPUT)) != 0) {
    LOGE(TAG, "Could not initialize GPIO %d, error = %d\n", LCD_GPIO_6, ret);
    return ret;
  }

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

  // ret = wifi_user_init();
  // if (ret)
  //   return ERR_WIFI_INIT;

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

  // ret = sensor_init();
  ret = weight_uart_485_init();
  if (ret)
    return 6;

  syslog_init();

  generate_syscfg();

  check_model();

  // sdcard_init();

  gpio_init_to_sc01_IO();

  i2s_speak_init();

  fm_init(PARTITION_NAME, BASE_PATH);

  set_file_log_number(9);

  // TODO : Temporary code for testing, the code below will be removed after implementing of espnow_add_peers()
  // Get a main mac address that will be used in espnow
  return SYSINIT_OK;
}

extern "C" {
void app_main(void) {
  int rc = SYSINIT_OK;

  if ((rc = system_init()) != SYSINIT_OK) {
    LOGE(TAG, "Failed to initialize device, error = [%d]", rc);
    return;
  }

  // Start interactive shell command line
  sc_ctx = sc_init();
  if (sc_ctx) {
    sc_start(sc_ctx);
  }
  syscfg_get(CFG_DATA, "MSC_OTA_MODE", msc_mode_check, sizeof(msc_mode_check));
  syscfg_get(CFG_DATA, "INDICATOR_MODEL", indicator_model_buf, sizeof(indicator_model_buf));

  if (lv_display_init() != 0) {
    LOGE(TAG, "LVGL setup failed!!!");
  }
  // usb msc or msc ota mode

  if (strncmp(msc_mode_check, "OTA", 3) == 0) {
    create_usb_host_msc_ota_task();
  } else if (strncmp(msc_mode_check, "MSC", 3) == 0) {
    create_usb_host_msc_task();
  }
  //  create_control_task();

  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
}
