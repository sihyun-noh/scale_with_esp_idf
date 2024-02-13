/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "lvgl.h"
#include "lcd_panel_rgb_gt911.h"
#include "config.h"

static const char *TAG = "main_app";

extern void example_lvgl_demo_ui(lv_disp_t *disp);

const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
static SemaphoreHandle_t xGuiSemaphore = NULL;
static TaskHandle_t g_lvgl_task_handle;

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

  xTaskCreatePinnedToCore(lv_tick_task, "lv_tick_task", 4096, NULL, 1, &g_lvgl_task_handle, 0);

  example_lvgl_demo_ui(disp);

  while (1) {
    vTaskDelay(xDelay);
    ESP_LOGI(TAG, "main_...");
  }
}
