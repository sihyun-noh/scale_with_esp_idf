#include <string.h>
#include "syscfg.h"
#include "sysevent.h"
#include "event_ids.h"
#include "syslog.h"
#include "gpio_api.h"
#include "config.h"

static TaskHandle_t actuator_handle = NULL;
static char model_name[10] = { 0 };

#if defined(ACT_BOARD_VER) && (ACT_BOARD_VER == ACT_OLIMEX_HW)
static uint8_t gpio_arry[] = { 2, 4, 32, 33, 14, 0, 13, 15, 16 };
#else
static uint8_t gpio_arry[] = { 2, 4, 32, 33, 14, 12, 13, 15, 16 };
#endif

static const char *TAG = "actuator";

typedef enum { INPUT = 0, INPUT_PULLUP = 1, OUTPUT = 2 } gpio_hal_mode_t;

static int relay_on(uint8_t pin);
static int relay_off(uint8_t pin);

int actuator_init(void) {
  int ret;
  syscfg_get(MFG_DATA, "model_name", model_name, sizeof(model_name));

  for (uint8_t i = 0; i < sizeof(gpio_arry); i++) {
    relay_off(gpio_arry[i]);
    if ((ret = gpio_init(gpio_arry[i], OUTPUT)) != 0) {
      LOGE(TAG, "%s : Could not initialize, error = %d\n", ret);
      return ret;
    }
    relay_off(gpio_arry[i]);
  }

#if defined(ACT_BOARD_VER) && (ACT_BOARD_VER == ACT_OLIMEX_HW)
  gpio_init(PIN_PHY_POWER, OUTPUT);
  gpio_write(PIN_PHY_POWER, 1);
#endif
  return 0;
}

static int relay_on(uint8_t pin) {
  int res;

  res = gpio_write(pin, 0);

  return res;
}

static int relay_off(uint8_t pin) {
  int res;

  res = gpio_write(pin, 1);

  return res;
}

static int actuator_all_off(void) {
  int res;

  for (uint8_t i = 0; i < sizeof(gpio_arry); i++) {
    res = relay_off(gpio_arry[i]);
    if (res) {
      return res;
    }
  }
  return res;
}

void test_gpio(void) {
  static char buff;
  static uint8_t control;
  uint8_t i = 0;

  switch (buff) {
    case 0:
      relay_on(gpio_arry[control]);
      LOGI(TAG, "gpio test : %d, control : %d", gpio_arry[control], control);
      if (control >= sizeof(gpio_arry) - 1) {
        control = 0;
      } else {
        control++;
      }
      buff++;
      break;
    case 1:
    default:
      for (i = 0; i < sizeof(gpio_arry); i++) {
        relay_off(gpio_arry[control]);
      }
      LOGI(TAG, "gpio test off");
      buff = 0;
      break;
  }
}

static void actuator_switch_action(void) {
  char s_actuator[20] = { 0 };

  if (sysevent_get("SYSEVENT_BASE", ACTUATOR_AUTO, &s_actuator, sizeof(s_actuator)) == 0) {
    if (!strncmp(s_actuator, "off", 3)) {
      actuator_all_off();
    } else if (!strncmp(s_actuator, "on", 2)) {
      relay_on(gpio_arry[8]);
      vTaskDelay(500 / portTICK_PERIOD_MS);
      for (uint8_t i = 0; i < 8; i++) {
        if (sysevent_get("SYSEVENT_BASE", ACTUATOR_PORT1 + i, &s_actuator, sizeof(s_actuator)) == 0) {
          if (!strncmp(s_actuator, "on", 2)) {
            relay_on(gpio_arry[i]);
          } else if (!strncmp(s_actuator, "off", 3)) {
            relay_off(gpio_arry[i]);
          }
          LOGI(TAG, "i : %d, s_actuator: %s, gpio_arry[i] : %d", i, s_actuator, gpio_arry[i]);
          vTaskDelay(500 / portTICK_PERIOD_MS);
        }
      }
    }
  }
}

static void actuator_motor_action(void) {
  char s_actuator[20] = { 0 };

  if (sysevent_get("SYSEVENT_BASE", ACTUATOR_AUTO, &s_actuator, sizeof(s_actuator)) == 0) {
    if (!strncmp(s_actuator, "off", 3)) {
      actuator_all_off();
    } else if (!strncmp(s_actuator, "on", 2)) {
      relay_on(gpio_arry[8]);
      vTaskDelay(500 / portTICK_PERIOD_MS);
      for (uint8_t i = 0; i < 4; i++) {
        if (sysevent_get("SYSEVENT_BASE", ACTUATOR_PORT1 + i, &s_actuator, sizeof(s_actuator)) == 0) {
          vTaskDelay(500 / portTICK_PERIOD_MS);
          if (!strncmp(s_actuator, "fwd", 3)) {
            relay_off(gpio_arry[i * 2]);
            relay_off(gpio_arry[(i * 2) + 1]);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            relay_on(gpio_arry[i * 2]);
            relay_off(gpio_arry[(i * 2) + 1]);
          } else if (!strncmp(s_actuator, "rwd", 3)) {
            relay_off(gpio_arry[i * 2]);
            relay_off(gpio_arry[(i * 2) + 1]);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            relay_off(gpio_arry[i * 2]);
            relay_on(gpio_arry[(i * 2) + 1]);
          } else if (!strncmp(s_actuator, "stop", 4)) {
            relay_off(gpio_arry[i * 2]);
            relay_off(gpio_arry[(i * 2) + 1]);
          }
          LOGI(TAG, "i : %d, s_port: %s, gpio_arry[n_port] : %d", i, s_actuator, gpio_arry[i]);
        }
      }
    }
  }
}

static void actuator_task(void *pvParameters) {
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    // test_gpio();
    if (!strncmp(model_name, "GLASW", 5)) {
      actuator_switch_action();
    } else if (!strncmp(model_name, "GLAMT", 5)) {
      actuator_motor_action();
    }
  }
}

void create_actuator_task(void) {
  uint16_t stack_size = 4096;
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (actuator_handle) {
    LOGI(TAG, "Sensor task is alreay created");
    return;
  }

  xTaskCreate((TaskFunction_t)actuator_task, "actuator_task", stack_size, NULL, task_priority, &actuator_handle);
}
