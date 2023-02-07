#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "syslog.h"
#include "sys_status.h"
#include "gpio_api.h"
#include "config.h"
#include "actuator.h"

static const char *TAG = "actuator";

int pump_init(void) {
  int ret;

  gpio_write(PUMP_ON, 1);
  if ((ret = gpio_init(PUMP_ON, OUTPUT)) != 0) {
    LOGE(TAG, "Could not initialize GPIO %d, error = %d\n", PUMP_ON, ret);
    set_actuator_err(1);
    return ret;
  }
  gpio_write(PUMP_ON, 1);
  set_actuator_err(0);

  return 0;
}

void pump_on(void) {
  gpio_write(PUMP_ON, 0);
  set_actuator_open(1);
}

void pump_off(void) {
  gpio_write(PUMP_ON, 1);
  set_actuator_open(0);
}
