#include <string.h>

#include "esp_event.h"
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

#include "control_task.h"

typedef enum { INPUT = 0, INPUT_PULLUP, OUTPUT } gpio_hal_mode_t;

static const char *TAG = "main_app";

sc_ctx_t *sc_ctx = NULL;

char usb_mode[5] = { 0 };
char speaker_set[5] = { 0 };
char printer_set[5] = { 0 };
char indicator_set[20] = { 0 };

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
  syscfg_get(SYSCFG_I_USB_MODE, SYSCFG_N_USB_MODE, usb_mode, sizeof(usb_mode));
  syscfg_get(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, indicator_set, sizeof(indicator_set));
  syscfg_get(SYSCFG_I_SPEAKER, SYSCFG_N_SPEAKER, speaker_set, sizeof(speaker_set));
  syscfg_get(SYSCFG_I_PRINTER, SYSCFG_N_PRINTER, printer_set, sizeof(printer_set));
  if (strcmp(usb_mode, "") == 0) {
    syscfg_get(MFG_DATA, SYSCFG_N_USB_MODE, usb_mode, sizeof(usb_mode));
    LOGI(TAG, "MFG GET usb_mode : %s", usb_mode);
    syscfg_set(SYSCFG_I_USB_MODE, SYSCFG_N_USB_MODE, usb_mode);
  }
  if (strcmp(indicator_set, "") == 0) {
    syscfg_get(MFG_DATA, SYSCFG_N_INDICATOR_SET, indicator_set, sizeof(indicator_set));
    LOGI(TAG, "MFG GET indicator_set : %s", indicator_set);
    syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, indicator_set);
  }
  if (strcmp(speaker_set, "") == 0) {
    syscfg_get(MFG_DATA, SYSCFG_N_SPEAKER, speaker_set, sizeof(speaker_set));
    LOGI(TAG, "MFG GET speaker_set : %s", speaker_set);
    syscfg_set(SYSCFG_I_SPEAKER, SYSCFG_N_SPEAKER, speaker_set);
  }
  if (strcmp(printer_set, "") == 0) {
    syscfg_get(MFG_DATA, SYSCFG_N_PRINTER, printer_set, sizeof(printer_set));
    LOGI(TAG, "MFG GET printer_set : %s", printer_set);
    syscfg_set(SYSCFG_I_SPEAKER, SYSCFG_N_PRINTER, printer_set);
  }

  LOGI(TAG, "usb_mode : %s, indicator_set : %s, speaker_set : %s, printer_set : %s", usb_mode, indicator_set,
       speaker_set, printer_set);
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

  if (lv_display_init() != 0) {
    LOGE(TAG, "LVGL setup failed!!!");
  }
  // usb msc or msc ota mode
  if (strncmp(usb_mode, "OTA", 3) == 0) {
    create_usb_host_msc_ota_task();
  } else if (strncmp(usb_mode, "MSC", 3) == 0) {
    create_usb_host_msc_task();
    create_control_task(indicator_set);
  }

  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
}
