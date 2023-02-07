#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "config.h"
#include "led.h"
#include "log.h"
#include "sys_status.h"
#include "syscfg.h"

static const char* TAG = "led_task";
static TaskHandle_t led_handle = NULL;

static void led_task(void* pvParameters) {
  led_off(LED_RED);
  led_off(LED_GREEN);
  led_off(LED_BLUE);
  if (led_init(LED_RED) != LED_OK) {
    LOGI(TAG, "Could not initialize Red LED");
  } else if (led_init(LED_GREEN) != LED_OK) {
    LOGI(TAG, "Could not initialize Green LED");
  } else if (led_init(LED_BLUE) != LED_OK) {
    LOGI(TAG, "Could not initialize Blue LED");
  } else {
    LOGI(TAG, "led_task start");
    while (1) {
      if (is_actuator_err()) {
        led_on(LED_RED);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        led_off(LED_RED);
      } else if (is_actuator_open()) {
        led_on(LED_GREEN);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        led_off(LED_GREEN);
      } else {
        led_on(LED_BLUE);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        led_off(LED_BLUE);
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
  vTaskDelete(NULL);
}

void create_led_task(void) {
  uint16_t stack_size = 8192;
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (led_handle) {
    LOGI(TAG, "LED task is alreay created");
    return;
  }

  xTaskCreate((TaskFunction_t)led_task, "LED", stack_size, NULL, task_priority, &led_handle);
}
