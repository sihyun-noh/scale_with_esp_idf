#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "syslog.h"
#include "sys_status.h"
#include "gpio_api.h"
#include "config.h"
#include "actuator.h"

static const char *TAG = "actuator";

int valve_init(void) {
  int ret;

  gpio_write(SOL_PW, 0);
  if ((ret = gpio_init(SOL_PW, OUTPUT)) != 0) {
    LOGE(TAG, "Could not initialize GPIO %d, error = %d\n", SOL_PW, ret);
    set_actuator_err(1);
    return ret;
  }
  gpio_write(SOL_PW, 0);

  gpio_write(SOL_ON, 1);
  if ((ret = gpio_init(SOL_ON, OUTPUT)) != 0) {
    LOGE(TAG, "Could not initialize GPIO %d, error = %d\n", SOL_ON, ret);
    set_actuator_err(1);
    return ret;
  }
  gpio_write(SOL_ON, 1);
  set_actuator_err(0);

  valve_close();

  return 0;
}

void valve_open(void) {
  gpio_write(SOL_ON, 0);
  vTaskDelay(10 / portTICK_PERIOD_MS);
  gpio_write(SOL_PW, 1);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  gpio_write(SOL_PW, 0);
  gpio_write(SOL_ON, 1);
  set_actuator_open(1);
}

void valve_close(void) {
  gpio_write(SOL_ON, 1);
  vTaskDelay(10 / portTICK_PERIOD_MS);
  gpio_write(SOL_PW, 1);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  gpio_write(SOL_PW, 0);
  set_actuator_open(0);
}
