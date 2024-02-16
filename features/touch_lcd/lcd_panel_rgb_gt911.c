/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 * seed fic
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "demos/lv_demos.h"
#include "driver/i2c.h"
#include "esp_lcd_touch_gt911.h"
#include "syslog.h"

static const char *TAG = "rgb_gt911_lcd";

// I2C settings
#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM \
  0 /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 400000   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (18 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL 1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_BK_LIGHT -1
#define EXAMPLE_PIN_NUM_HSYNC 46
#define EXAMPLE_PIN_NUM_VSYNC 3
#define EXAMPLE_PIN_NUM_DE 5
#define EXAMPLE_PIN_NUM_PCLK 7
#define EXAMPLE_PIN_NUM_DATA0 14   // B3
#define EXAMPLE_PIN_NUM_DATA1 38   // B4
#define EXAMPLE_PIN_NUM_DATA2 18   // B5
#define EXAMPLE_PIN_NUM_DATA3 17   // B6
#define EXAMPLE_PIN_NUM_DATA4 10   // B7
#define EXAMPLE_PIN_NUM_DATA5 39   // G2
#define EXAMPLE_PIN_NUM_DATA6 0    // G3
#define EXAMPLE_PIN_NUM_DATA7 45   // G4
#define EXAMPLE_PIN_NUM_DATA8 48   // G5
#define EXAMPLE_PIN_NUM_DATA9 47   // G6
#define EXAMPLE_PIN_NUM_DATA10 21  // G7
#define EXAMPLE_PIN_NUM_DATA11 1   // R3
#define EXAMPLE_PIN_NUM_DATA12 2   // R4
#define EXAMPLE_PIN_NUM_DATA13 42  // R5
#define EXAMPLE_PIN_NUM_DATA14 41  // R6
#define EXAMPLE_PIN_NUM_DATA15 40  // R7
#define EXAMPLE_PIN_NUM_DISP_EN -1

// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES CONFIG_LCD_H_RES
#define EXAMPLE_LCD_V_RES CONFIG_LCD_V_RES

#define EXAMPLE_LVGL_TICK_PERIOD_MS 2

#if CONFIG_EXAMPLE_DOUBLE_FB
#define EXAMPLE_LCD_NUM_FB 2
#else
#define EXAMPLE_LCD_NUM_FB 1
#endif  // CONFIG_EXAMPLE_DOUBLE_FB

// we use two semaphores to sync the VSYNC event and the LVGL task, to avoid potential tearing effect

lv_disp_t *disp = NULL;
static esp_lcd_touch_handle_t tp = NULL;

#if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
SemaphoreHandle_t sem_vsync_end;
SemaphoreHandle_t sem_gui_ready;
#endif
// LV_IMG_DECLARE(img_480_800_16_bmp);

static bool example_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data,
                                   void *user_data) {
  BaseType_t high_task_awoken = pdFALSE;
#if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
  if (xSemaphoreTakeFromISR(sem_gui_ready, &high_task_awoken) == pdTRUE) {
    xSemaphoreGiveFromISR(sem_vsync_end, &high_task_awoken);
  }
#endif
  return high_task_awoken == pdTRUE;
}

static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
  esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
  int offsetx1 = area->x1;
  int offsetx2 = area->x2;
  int offsety1 = area->y1;
  int offsety2 = area->y2;
#if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
  xSemaphoreGive(sem_gui_ready);
  xSemaphoreTake(sem_vsync_end, portMAX_DELAY);
#endif
  // pass the draw buffer to the driver
  esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
  lv_disp_flush_ready(drv);
}

/*Read the touchpad*/
void example_touchpad_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  uint16_t touchpad_x[1] = { 0 };
  uint16_t touchpad_y[1] = { 0 };
  uint8_t touchpad_cnt = 0;

  /* Read touch controller data */
  esp_lcd_touch_read_data(drv->user_data);

  /* Get coordinates */
  bool touchpad_pressed = esp_lcd_touch_get_coordinates(drv->user_data, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);

  if (touchpad_pressed && touchpad_cnt > 0) {
    data->point.x = touchpad_x[0];
    data->point.y = touchpad_y[0];
    data->state = LV_INDEV_STATE_PR;
    ESP_LOGI(TAG, "X=%u Y=%u", data->point.x, data->point.y);
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

static void lv_tick_inc_cb(void *data) {
  uint32_t tick_inc_period_ms = *((uint32_t *)data);
  lv_tick_inc(tick_inc_period_ms);
}

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void) {
  int i2c_master_port = I2C_MASTER_NUM;

  i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_MASTER_SDA_IO,
    .scl_io_num = I2C_MASTER_SCL_IO,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = I2C_MASTER_FREQ_HZ,
  };

  i2c_param_config(i2c_master_port, &conf);

  return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void lv_port_12c_tp_init() {
  ESP_ERROR_CHECK(i2c_master_init());
  ESP_LOGI(TAG, "I2C initialized successfully");

  esp_lcd_panel_io_handle_t tp_io_handle = NULL;
  esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
  ESP_LOGI(TAG, "Initialize touch IO (I2C)");
  /* Touch IO handle */
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)I2C_MASTER_NUM, &tp_io_config, &tp_io_handle));
  esp_lcd_touch_config_t tp_cfg = {
        .x_max = EXAMPLE_LCD_V_RES,
        .y_max = EXAMPLE_LCD_H_RES,
        .rst_gpio_num = -1,
        .int_gpio_num = 4,
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
  /* Initialize touch */
  ESP_LOGI(TAG, "Initialize touch controller GT911");
  ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &tp));

  int ret;
  uint8_t write_buf = 0x01;

  ret = i2c_master_write_to_device(I2C_MASTER_NUM, 0x24, &write_buf, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
  ESP_LOGI(TAG, "0x48 0x01 ret is %d", ret);

  write_buf = 0x0E;
  ret = i2c_master_write_to_device(I2C_MASTER_NUM, 0x38, &write_buf, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
  ESP_LOGI(TAG, "0x70 0x00 ret is %d", ret);
}

// int lcd_panel_rgb16bit_gt911(void (*main_ui_init)(lv_disp_t *disp)) {
void lv_port_disp_init(void) {
  static lv_disp_draw_buf_t disp_buf;  // contains internal graphic buffer(s) called draw buffer(s)
  static lv_disp_drv_t disp_drv;       // contains callback functions

#if CONFIG_EXAMPLE_AVOID_TEAR_EFFECT_WITH_SEM
  ESP_LOGI(TAG, "Create semaphores");
  sem_vsync_end = xSemaphoreCreateBinary();
  assert(sem_vsync_end);
  sem_gui_ready = xSemaphoreCreateBinary();
  assert(sem_gui_ready);
#endif

#if EXAMPLE_PIN_NUM_BK_LIGHT >= 0
  ESP_LOGI(TAG, "Turn off LCD backlight");
  gpio_config_t bk_gpio_config = { .mode = GPIO_MODE_OUTPUT, .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT };
  ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
#endif

  ESP_LOGI(TAG, "Install RGB LCD panel driver");
  esp_lcd_panel_handle_t panel_handle = NULL;
  esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = 16, // RGB565 in parallel mode, thus 16bit in width
        .psram_trans_align = 64,
        .num_fbs = EXAMPLE_LCD_NUM_FB,
#if CONFIG_EXAMPLE_USE_BOUNCE_BUFFER
        .bounce_buffer_size_px = 10 * EXAMPLE_LCD_H_RES,
#endif
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .disp_gpio_num = EXAMPLE_PIN_NUM_DISP_EN,
        .pclk_gpio_num = EXAMPLE_PIN_NUM_PCLK,
        .vsync_gpio_num = EXAMPLE_PIN_NUM_VSYNC,
        .hsync_gpio_num = EXAMPLE_PIN_NUM_HSYNC,
        .de_gpio_num = EXAMPLE_PIN_NUM_DE,
        .data_gpio_nums = {
            EXAMPLE_PIN_NUM_DATA0,
            EXAMPLE_PIN_NUM_DATA1,
            EXAMPLE_PIN_NUM_DATA2,
            EXAMPLE_PIN_NUM_DATA3,
            EXAMPLE_PIN_NUM_DATA4,
            EXAMPLE_PIN_NUM_DATA5,
            EXAMPLE_PIN_NUM_DATA6,
            EXAMPLE_PIN_NUM_DATA7,
            EXAMPLE_PIN_NUM_DATA8,
            EXAMPLE_PIN_NUM_DATA9,
            EXAMPLE_PIN_NUM_DATA10,
            EXAMPLE_PIN_NUM_DATA11,
            EXAMPLE_PIN_NUM_DATA12,
            EXAMPLE_PIN_NUM_DATA13,
            EXAMPLE_PIN_NUM_DATA14,
            EXAMPLE_PIN_NUM_DATA15,
        },
        .timings = {
            .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
            .h_res = EXAMPLE_LCD_H_RES,
            .v_res = EXAMPLE_LCD_V_RES,
            // The following parameters should refer to LCD spec
            .hsync_back_porch = 30,
            .hsync_front_porch = 210,
            .hsync_pulse_width = 30,
            .vsync_back_porch = 4,
            .vsync_front_porch = 4,
            .vsync_pulse_width = 4,
            .flags.pclk_active_neg = true,
        },
        .flags.fb_in_psram = true, // allocate frame buffer in PSRAM
    };
  ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));

  ESP_LOGI(TAG, "Register event callbacks");
  esp_lcd_rgb_panel_event_callbacks_t cbs = {
    .on_vsync = example_on_vsync_event,
  };
  ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, &disp_drv));

  ESP_LOGI(TAG, "Initialize RGB LCD panel");
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

#if EXAMPLE_PIN_NUM_BK_LIGHT >= 0
  ESP_LOGI(TAG, "Turn on LCD backlight");
  gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
#endif

  void *buf1 = NULL;
  void *buf2 = NULL;
#if CONFIG_EXAMPLE_DOUBLE_FB
  ESP_LOGI(TAG, "Use frame buffers as LVGL draw buffers");
  ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &buf1, &buf2));
  // initialize LVGL draw buffers
  lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES);
#else
  ESP_LOGI(TAG, "Allocate separate LVGL draw buffers from PSRAM");
  buf1 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 160 * sizeof(lv_color_t), MALLOC_CAP_DMA);
  assert(buf1);
  // buf2 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 80 * sizeof(lv_color_t), MALLOC_CAP_DMA);
  // assert(buf2);
  // initialize LVGL draw buffers
  lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * 160);
#endif  // CONFIG_EXAMPLE_DOUBLE_FB

  ESP_LOGI(TAG, "Register display driver to LVGL");
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = EXAMPLE_LCD_H_RES;
  disp_drv.ver_res = EXAMPLE_LCD_V_RES;
  disp_drv.flush_cb = example_lvgl_flush_cb;
  disp_drv.draw_buf = &disp_buf;
  disp_drv.user_data = panel_handle;

#if CONFIG_EXAMPLE_DOUBLE_FB
  disp_drv.full_refresh = true;  // the full_refresh mode can maintain the synchronization between the two frame buffers
#endif
  lv_disp_drv_register(&disp_drv);
}

void lv_port_indev_init(void) {
  ESP_LOGI(TAG, "Register display indev to LVGL");
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.disp = disp;
  indev_drv.read_cb = example_touchpad_read;
  indev_drv.user_data = tp;
  lv_indev_drv_register(&indev_drv);
}

esp_err_t lv_port_tick_init(void) {
  static const uint32_t tick_inc_period_ms = EXAMPLE_LVGL_TICK_PERIOD_MS;
  ESP_LOGI(TAG, "Install LVGL tick timer");

  /* Create and start a periodic timer interrupt to call lv_tick_inc */
  const esp_timer_create_args_t lv_periodic_timer_args = { .callback = &lv_tick_inc_cb,
                                                           .arg = &tick_inc_period_ms,
                                                           .dispatch_method = ESP_TIMER_TASK,
                                                           .name = "periodic_gui",
                                                           .skip_unhandled_events = true };
  esp_timer_handle_t lv_periodic_timer;
  ESP_ERROR_CHECK(esp_timer_create(&lv_periodic_timer_args, &lv_periodic_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(lv_periodic_timer, tick_inc_period_ms * 1000));
  return ESP_OK;
}
