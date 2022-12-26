#include "freertos/projdefs.h"
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
#include "sysfile.h"
#include "config.h"
#include "main.h"
#include "esp_now.h"

#include <string.h>

#define LV_TICK_PERIOD_MS 1
#define LGFX_USE_V1  // LovyanGFX version

#include "lvgl.h"
#include "LovyanGFX.h"
#include "conf_WT32SCO1-Plus.h"

#include "ui.h"

static LGFX lcd;  // LCD Driver

/* Setup screen resolution for LVGL */
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

/* Function declaration */
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);

static void lv_tick_task(void *arg);
static void gui_task(void *pvParameter);

// static void main_ctrl_app(void);

static void lv_tick_task(void *arg);
const char *TAG = "main_app";

sc_ctx_t *sc_ctx = NULL;

extern "C" {
extern int read_battery_percentage(void);
extern void sdcard_init(void);
extern void sdcard_write_data(void);
// extern void create_led_task(void);
}

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
extern "C" void stop_shell(void) {
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

int set_interval_cmd(int argc, char **argv) {
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

int get_interval_cmd(int argc, char **argv) {
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

  ret = wifi_user_init();
  if (ret)
    return ERR_WIFI_INIT;

  ret = wifi_espnow_mode();
  if (ret)
    return ERR_WIFI_INIT;

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

  // create_led_task();

  return SYSINIT_OK;
}

void loop_task(void) {
  set_operation_mode(ESP_NOW_INIT_MODE);

  while (1) {
    switch (get_operation_mode()) {
      case ESP_NOW_INIT_MODE: {
        LOGI(TAG, "ESP-NOW INIT MODE");
        if (esp_now_init()) {
          LOGE(TAG, "ESP_ERR_ESPNOW_INTERNAL");
          set_operation_mode(DEEP_SLEEP_MODE);
        }
        set_operation_mode(HID_INIT_MODE);
      } break;
      case HID_INIT_MODE: {
        // LOGI(TAG, "HID_INIT_MODE");
        set_operation_mode(HID_READ_MODE);
      } break;
      case HID_READ_MODE: {
        // LOGI(TAG, "HID_READ_MODE");
        set_operation_mode(HID_DISPLAY_MODE);
      } break;
      case HID_DISPLAY_MODE: {
        // LOGI(TAG, "HID_DISPLAY_MODE");
        set_operation_mode(DEEP_SLEEP_MODE);
      } break;
      case DEEP_SLEEP_MODE: {
        // LOGI(TAG, "DEEP_SLEEP_MODE");
        //  vTaskDelay(send_interval * 1000 / portTICK_PERIOD_MS);
        set_operation_mode(HID_READ_MODE);
      } break;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// Setting up tick task for lvgl
static void lv_tick_task(void *arg) {
  (void)arg;
  lv_tick_inc(LV_TICK_PERIOD_MS);
}

// Display callback to flush the buffer to screen
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, w, h);
  lcd.pushColors((uint16_t *)&color_p->full, w * h, true);
  lcd.endWrite();

  lv_disp_flush_ready(disp);
}

// Touchpad callback to read the touchpad
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  bool touched = lcd.getTouch(&touchX, &touchY);

  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;

    // sprintf(txt, "Touch:(%03d,%03d)", touchX, touchY);
    // lv_label_set_text(tlabel, txt); // set label text
  }
}

/* Creates a semaphore to handle concurrent call to lvgl stuff
 * If you wish to call *any* lvgl function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xGuiSemaphore;

static void gui_task(void *pvParameter) {
  (void)pvParameter;

  xGuiSemaphore = xSemaphoreCreateMutex();

  // Initialize LovyanGFX
  lcd.init();

  // Initialize lvgl
  lv_init();

  // Setting display to landscape
  lcd.setRotation(lcd.getRotation() ^ 1);

  // LVGL : Setting up buffer to use for display
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  // LVGL : Setup and Initialize the display device driver
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = display_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // LVGL : Setup & Initialize the input device driver
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read;
  lv_indev_drv_register(&indev_drv);

  /* Create and start a periodic timer interrupt to call lv_tick_inc */
  const esp_timer_create_args_t periodic_timer_args = { .callback = &lv_tick_task, .name = "periodic_gui" };
  esp_timer_handle_t periodic_timer;
  ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

  ui_init();

  while (1) {
    // Delay 1 tick (FreeRTOS tick is 10ms)
    vTaskDelay(pdMS_TO_TICKS(10));

    // Try to take the semaphore, call lvgl related function on success
    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
      lv_timer_handler();
      xSemaphoreGive(xGuiSemaphore);
    }
  }

  vTaskDelete(NULL);
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

  xTaskCreatePinnedToCore(gui_task, "gui", 4096 * 2, NULL, tskIDLE_PRIORITY, NULL, 0);

  loop_task();
}
}
