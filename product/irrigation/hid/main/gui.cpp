#include "freertos/portmacro.h"
#define LV_TICK_PERIOD_MS 1
#define LGFX_USE_V1  // LovyanGFX version

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_timer.h>

#include "lvgl.h"
#include "LovyanGFX.h"
#include "conf_WT32SCO1-Plus.h"

#include "log.h"
#include "gui.h"
#include "sys_status.h"
#include "ui_helpers.h"

#define BUFF_SIZE 40
// #define BUFF_SIZE 10
// #define LVGL_STATIC_BUFFER
#define LVGL_DOUBLE_BUFFER

static const char *tag = "GUI";

/* Setup screen resolution for LVGL */
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_disp_t *disp;

static LGFX lcd;  // LCD driver

#if defined(LVGL_STATIC_BUFFER)
static lv_color_t buf1[screenWidth * screenHeight / BUFF_SIZE];
#endif

/* Creates a semaphore to handle concurrent call to lvgl stuff
 * If you wish to call *any* lvgl function from other threads/tasks
 * you should lock on the very same semaphore! */
static SemaphoreHandle_t xGuiSemaphore = NULL;
static TaskHandle_t g_lvgl_task_handle;

static void gui_task(void *args);
static void lv_tick_task(void *arg);

/*** Function declaration ***/
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);

/* Setting up tick task for lvgl */
static void lv_tick_task(void *arg) {
  (void)arg;
  lv_tick_inc(LV_TICK_PERIOD_MS);
}

static void gui_task(void *args) {
  LOGI(tag, "Start to run LVGL");
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(10));

    /* Try to take the semaphore, call lvgl related function on success */
    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
      lv_task_handler();
      // lv_timer_handler_run_in_period(5); /* run lv_timer_handler() every 5ms */
      xSemaphoreGive(xGuiSemaphore);
    }
  }
}

int lv_display_init(void) {
  // Initialize LovyanGFX
  lcd.init();

  // Init DMA
  // lcd.initDMA();

  // Initialize lvgl
  lv_init();

  // Setting display to landscape
  lcd.setRotation(lcd.getRotation() ^ 1);
  // lcd.setColorDepth(16);
  // lcd.setBrightness(128);

#if defined(LVGL_STATIC_BUFFER)
  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, screenWidth * screenHeight / BUFF_SIZE);
#else
#if defined(LVGL_DOUBLE_BUFFER)
  lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(screenWidth * BUFF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
  lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(screenWidth * BUFF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);

  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, screenWidth * BUFF_SIZE);
#else
  lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(screenWidth * BUFF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, screenWidth * BUFF_SIZE);
#endif
#endif

  // LVGL : Setup & Initialize the display device driver
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = display_flush;
  disp_drv.draw_buf = &draw_buf;
  disp = lv_disp_drv_register(&disp_drv);

  // LVGL : Setup & Initialize the input device driver
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read;
  lv_indev_drv_register(&indev_drv);

  /* Create and start a periodic timer interrupt to call lv_tick_inc */
  const esp_timer_create_args_t lv_periodic_timer_args = { .callback = &lv_tick_task,
                                                           .arg = NULL,
                                                           .dispatch_method = ESP_TIMER_TASK,
                                                           .name = "periodic_gui",
                                                           .skip_unhandled_events = true };
  esp_timer_handle_t lv_periodic_timer;
  ESP_ERROR_CHECK(esp_timer_create(&lv_periodic_timer_args, &lv_periodic_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(lv_periodic_timer, LV_TICK_PERIOD_MS * 1000));

  // Setup UI screen and theme
  ui_init();

  xGuiSemaphore = xSemaphoreCreateMutex();
  if (!xGuiSemaphore) {
    LOGE(tag, "Create mutex for LVGL failed");
    if (lv_periodic_timer) {
      esp_timer_delete(lv_periodic_timer);
    }
    return -1;
  }

  int err = xTaskCreatePinnedToCore(gui_task, "gui_task", 1024 * 8, NULL, 5, &g_lvgl_task_handle, 0);
  if (!err) {
    LOGE(tag, "Create task for LVGL failed");
    if (lv_periodic_timer) {
      esp_timer_delete(lv_periodic_timer);
    }
    return -1;
  }

  return 0;
}

// Display callback to flush the buffer to screen
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  /* Without DMA */
  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, w, h);
  lcd.pushPixels((uint16_t *)&color_p->full, w * h, true);
  lcd.endWrite();

  /* With DMA */
  // TODO: Yet to do performance test
  // lcd.startWrite();
  // lcd.setAddrWindow(area->x1, area->y1, w, h);
  // lcd.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1,
  //                 (lgfx::swap565_t *)&color_p->full);
  // lcd.endWrite();

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

    // Set the coordinates
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

bool lvgl_acquire(void) {
  TaskHandle_t task = xTaskGetCurrentTaskHandle();
  if (g_lvgl_task_handle != task) {
    return (xSemaphoreTake(xGuiSemaphore, 1000) == pdTRUE);
  }
  return false;
}

void lvgl_release(void) {
  TaskHandle_t task = xTaskGetCurrentTaskHandle();
  if (g_lvgl_task_handle != task) {
    xSemaphoreGive(xGuiSemaphore);
  }
}
