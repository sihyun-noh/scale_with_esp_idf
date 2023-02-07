#include <string.h>

#include "config.h"
#include "utils.h"
#include "adc_api.h"
#include "gpio_api.h"
#include "syslog.h"
#include "sysevent.h"
#include "event_ids.h"
#include "filelog.h"
#include "sys_status.h"

#define MAX_BUFFER_CNT 10

static const char* TAG = "battery_task";

enum { INPUT = 0, INPUT_PULLUP = 1, OUTPUT = 2 };

static float battery_calculate_percentage(uint16_t voltage);
static void battery_read_on();
static void battery_read_off();

void battery_init(void) {
  adc_init(BATTERY_ADC_CHANNEL);
  gpio_init(BATTERY_READ_ON_GPIO, OUTPUT);
}

/**
 * @brief Read the battery and pass it to sysevent
 *
 * @return {battery level}
 */
int read_battery_percentage(void) {
  uint16_t voltage;
  uint16_t battery_voltage[MAX_BUFFER_CNT];
  float bat_percent;
  char s_bat_percent[20];

  battery_read_on();
  vTaskDelay(500 / portTICK_PERIOD_MS);
  for (int i = 0; i < MAX_BUFFER_CNT; i++) {
    battery_voltage[i] = adc_read_voltage(BATTERY_ADC_CHANNEL) * 2;
    // LOGI(TAG, "cali data: %d mV", battery_voltage[i]);

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }

  battery_read_off();

  qsort(battery_voltage, MAX_BUFFER_CNT, sizeof(uint16_t), uint16_compare);
  voltage = uint16_average(&battery_voltage[1], MAX_BUFFER_CNT - 2);
  bat_percent = battery_calculate_percentage(voltage);
  LOGI(TAG, "battery percent: %.2f", bat_percent);

  memset(s_bat_percent, 0, sizeof(s_bat_percent));
  snprintf(s_bat_percent, sizeof(s_bat_percent), "%.2f", bat_percent);

  if (bat_percent < 20) {
    set_low_battery(1);
  }

  sysevent_set(ADC_BATTERY_EVENT, (char*)s_bat_percent);

  return (int)(bat_percent);
}

/**
 * @brief Calculate the battery value as a percentage
 *
 * @param voltage battery voltage
 *
 * @return percent value
 */
#define MAX_BATTERY 4200
#define LOW_BATTERY 2700
static float battery_calculate_percentage(uint16_t voltage) {
  float percent;

  if (voltage >= MAX_BATTERY) {
    percent = 100;
  } else if (voltage < LOW_BATTERY) {
    percent = 0;
  } else {
    percent = ((float)(voltage - LOW_BATTERY) / (MAX_BATTERY - LOW_BATTERY)) * 100;
  }

  return percent;
}

static void battery_read_on() {
  gpio_write(BATTERY_READ_ON_GPIO, 1);
}

static void battery_read_off() {
  gpio_write(BATTERY_READ_ON_GPIO, 0);
}
