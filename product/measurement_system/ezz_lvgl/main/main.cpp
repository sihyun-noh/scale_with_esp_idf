/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <cstdint>
#include <cstring>
#include <ctype.h>
#include <stdbool.h>
#include <inttypes.h>

#include "freertos/projdefs.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "lvgl.h"
#include "lcd_panel_rgb_gt911.h"
// #include "config.h"
#include "mqtt.h"
#include "CAN_comn.h"
#include "sibi_monitor.h"
#include "upper_task.h"
#include "ui/eez_agmo/src/ui/ui.h"
#include "ui_bridge.h"
#include "ui_adaptor.h"
#include "ui_struct_bridge.h"
#include "math_rpm.h"
#include "ui_msg.h"

static const char *TAG = "main_app";

bool lvgl_acquire(void);
void lvgl_release(void);
const TickType_t xDelay = 2000 / portTICK_PERIOD_MS;
static SemaphoreHandle_t xGuiSemaphore = NULL;
static TaskHandle_t eez_lv_tick_task_handle;

bool lvgl_acquire(void) {
  TaskHandle_t task = xTaskGetCurrentTaskHandle();
  if (eez_lv_tick_task_handle != task) {
    return (xSemaphoreTake(xGuiSemaphore, 1000) == pdTRUE);
  }
  return false;
}

void lvgl_release(void) {
  TaskHandle_t task = xTaskGetCurrentTaskHandle();
  if (eez_lv_tick_task_handle != task) {
    xSemaphoreGive(xGuiSemaphore);
  }
}

// lvgl ui task base code
void lv_tick_task(void *arg) {
  while (1) {
    // raise the task priority of LVGL and/or reduce the handler period can improve the performance
    vTaskDelay(pdMS_TO_TICKS(5));
    // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
      lv_task_handler();
      // lv_timer_handler_run_in_period(5); /* run lv_timer_handler() every 5ms */
      xSemaphoreGive(xGuiSemaphore);
    }
  }
}

void eez_lv_tick_task(void *arg) {
  ui_init();
  static const bridge_t *queue = bridge_get();

  ESP_LOGI(TAG, "eez start");
  while (1) {
    // raise the task priority of LVGL and/or reduce the handler period can improve the performance
    vTaskDelay(pdMS_TO_TICKS(5));
    ui_tick();
    // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
      lv_task_handler();
      // lv_timer_handler_run_in_period(5); /* run lv_timer_handler() every 5ms */
      xSemaphoreGive(xGuiSemaphore);
    }

    ui_msg_t msg;
    // hw -> ui value set
    while (xQueueReceive(queue->ui_q, &msg, 0) == pdTRUE) {
      if (msg.id == UI_EVT_FB_DRIVER1_R) {
        ESP_LOGE(TAG, "right : %s", msg.str);
        set_var_fb_driver1_r(msg.str);
      }
      if (msg.id == UI_EVT_FB_DRIVER1_L) {
        ESP_LOGE(TAG, "left : %s", msg.str);
        set_var_fb_driver1_l(msg.str);
      }

      if (msg.id == UI_EVT_FB_DRIVER2_R)
        set_var_fb_driver2_r(msg.str);

      if (msg.id == UI_EVT_FB_DRIVER2_L)
        set_var_fb_driver2_l(msg.str);

      if (msg.id == UI_EVT_VCU_STATUS_D0)
        ui_struct_set_field(0, msg.str);

      if (msg.id == UI_EVT_VCU_STATUS_D1)
        ui_struct_set_field(1, msg.str);

      if (msg.id == UI_EVT_VCU_STATUS_D2)
        ui_struct_set_field(2, msg.str);

      if (msg.id == UI_EVT_VCU_STATUS_D3)
        ui_struct_set_field(3, msg.str);

      if (msg.id == UI_EVT_VCU_STATUS_D4)
        ui_struct_set_field(4, msg.str);

      if (msg.id == UI_EVT_VCU_STATUS_D5)
        ui_struct_set_field(5, msg.str);

      if (msg.id == UI_EVT_VCU_STATUS_D6)
        ui_struct_set_field(6, msg.str);

      if (msg.id == UI_EVT_VCU_STATUS_D7)
        ui_struct_set_field(7, msg.str);
    }
  }
}

extern "C" {
void app_main(void) {
  xGuiSemaphore = xSemaphoreCreateMutex();

  ESP_LOGI(TAG, "main_...");
  ESP_LOGI(TAG, "Initialize LVGL library");
  lv_init();
  lv_port_disp_init();
  lv_port_12c_tp_init();
  lv_port_indev_init();
  lv_port_tick_init();
  ESP_LOGI(TAG, "init done");

  QueueHandle_t ui_q = xQueueCreate(32, sizeof(ui_msg_t));
  QueueHandle_t hw_q = xQueueCreate(32, sizeof(ui_msg_t));

  bridge_init_singleton(ui_q, hw_q);

  // 이후 UI task, HW task 생성

  /* clang-format off */

  // Create the EEZ UI tick task pinned to core 0.
//
// This task drives the EEZ-generated UI runtime on top of LVGL.
// Responsibilities:
// - Calls eez_ui_init() once to initialize the EEZ UI (create initial screens,
//   bind native variables, setup flows/actions).
// - Periodically calls eez_ui_tick() to advance EEZ flows/actions/timers and
//   propagate UI logic changes.
// - Typically runs alongside lv_timer_handler() (either inside this task or in
//   a separate LVGL task). If LVGL is not pumped elsewhere, this task should
//   also call lv_timer_handler() to keep rendering/input/animations alive.
//
// Note: Keep all EEZ/LVGL interactions in this single task (or protect with a mutex)
// to avoid multi-thread access to LVGL objects.
xTaskCreatePinnedToCore(
    eez_lv_tick_task,
    "eez_lv_tick_task",
    8192,
    NULL,
    5,
    &eez_lv_tick_task_handle,
    1
);

  /* clang-format on */

#if 1
  vTaskDelay(pdMS_TO_TICKS(100));
  ESP_ERROR_CHECK(waveshare_twai_init());  // Initialize the TWAI interface and check for errors
#if CONFIG_APP_RUN_MODE_UPPER
  upper_process_run();
#else
  sibi_process_run();
#endif

#endif
  while (1) {
    vTaskDelay(xDelay);
  }
}
}
