#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "syslog.h"
#include "gpio_api.h"
#include "config.h"
#include "actuator.h"

static const char *TAG = "actuator";

int valve_init(void) {
  int ret;

  gpio_write(SOL_PW, 0);
  if ((ret = gpio_init(SOL_PW, OUTPUT)) != 0) {
    LOGE(TAG, "Could not initialize GPIO %d, error = %d\n", SOL_PW, ret);
    return ret;
  }
  gpio_write(SOL_PW, 0);

  gpio_write(SOL_ON, 1);
  if ((ret = gpio_init(SOL_ON, OUTPUT)) != 0) {
    LOGE(TAG, "Could not initialize GPIO %d, error = %d\n", SOL_ON, ret);
    return ret;
  }
  gpio_write(SOL_ON, 1);

  return 0;
}

int valve_open(void) {
  int res;

  res = gpio_write(SOL_PW, 1);
  if (res)
    return res;

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  res = gpio_write(SOL_ON, 0);

  return res;
}

int valve_close(void) {
  int res;

  res = gpio_write(SOL_PW, 0);
  if (res)
    return res;

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  res = gpio_write(SOL_ON, 1);

  return res;
}

int pump_init(void) {
  int ret;

  gpio_write(PUMP_ON, 1);
  if ((ret = gpio_init(PUMP_ON, OUTPUT)) != 0) {
    LOGE(TAG, "Could not initialize GPIO %d, error = %d\n", PUMP_ON, ret);
    return ret;
  }
  gpio_write(PUMP_ON, 1);

  return 0;
}

int pump_on(void) {
  int res;

  res = gpio_write(PUMP_ON, 0);

  return res;
}

int pump_off(void) {
  int res;

  res = gpio_write(PUMP_ON, 1);

  return res;
}
