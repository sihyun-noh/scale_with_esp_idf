#include <string.h>
#include "syscfg.h"
#include "sys_config.h"
#include "sysevent.h"
#include "event_ids.h"
#include "syslog.h"
#include "gpio_api.h"

static char model_name[10] = { 0 };

#define ACT_OLIMEX_HW 10
#define ACT_GLS_HW 20 /* Greenlabs HW */
#define ACT_BOARD_VER ACT_GLS_HW

#if defined(ACT_BOARD_VER) && (ACT_BOARD_VER == ACT_OLIMEX_HW)
#define PIN_PHY_POWER 12
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
  syscfg_get(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, model_name, sizeof(model_name));

  for (uint8_t i = 0; i < sizeof(gpio_arry); i++) {
    relay_off(gpio_arry[i]);
    if ((ret = gpio_init(gpio_arry[i], OUTPUT)) != 0) {
      LOGE(TAG, "Could not initialize, error = %d\n", ret);
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

  relay_on(gpio_arry[8]);
  switch (buff) {
    case 0:
      relay_on(gpio_arry[control]);
      LOGI(TAG, "gpio test : %d, control : %d", gpio_arry[control], control);
      control++;
      if (control >= sizeof(gpio_arry) - 1) {
        control = 0;
      }
      buff++;
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      break;
    case 1:
    default:
      for (i = 0; i < sizeof(gpio_arry) - 1; i++) {
        relay_off(gpio_arry[i]);
      }
      LOGI(TAG, "gpio test off");
      buff = 0;
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      break;
  }
}

#if (CONFIG_ACTUATOR_SWITCH)
static void actuator_switch_action(void) {
  char s_actuator[20] = { 0 };

  if (sysevent_get(SYSEVENT_BASE, ACTUATOR_AUTO, &s_actuator, sizeof(s_actuator)) == 0) {
    if (!strncmp(s_actuator, "off", 3)) {
      actuator_all_off();
    } else if (!strncmp(s_actuator, "on", 2)) {
      relay_on(gpio_arry[8]);
      vTaskDelay(500 / portTICK_PERIOD_MS);
      for (uint8_t i = 0; i < 8; i++) {
        if (sysevent_get(SYSEVENT_BASE, ACTUATOR_PORT1 + i, &s_actuator, sizeof(s_actuator)) == 0) {
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
#elif (CONFIG_ACTUATOR_MOTOR)
static void actuator_motor_action(void) {
  char s_actuator[20] = { 0 };

  if (sysevent_get(SYSEVENT_BASE, ACTUATOR_AUTO, &s_actuator, sizeof(s_actuator)) == 0) {
    if (!strncmp(s_actuator, "off", 3)) {
      actuator_all_off();
    } else if (!strncmp(s_actuator, "on", 2)) {
      relay_on(gpio_arry[8]);
      vTaskDelay(500 / portTICK_PERIOD_MS);
      for (uint8_t i = 0; i < 4; i++) {
        if (sysevent_get(SYSEVENT_BASE, ACTUATOR_PORT1 + i, &s_actuator, sizeof(s_actuator)) == 0) {
          vTaskDelay(500 / portTICK_PERIOD_MS);
          if (!strncmp(s_actuator, "open", 4)) {
            relay_off(gpio_arry[i * 2]);
            relay_off(gpio_arry[(i * 2) + 1]);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            relay_on(gpio_arry[i * 2]);
            relay_off(gpio_arry[(i * 2) + 1]);
          } else if (!strncmp(s_actuator, "close", 5)) {
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
#endif

void actuator_task(void) {
#if (CONFIG_ACTUATOR_SWITCH)
  actuator_switch_action();
#elif (CONFIG_ACTUATOR_MOTOR)
  actuator_motor_action();
#endif
}
