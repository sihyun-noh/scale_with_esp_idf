#include "config.h"
#include "battery.h"
#include "utils.h"

#include <string.h>

#include "syslog.h"
#include "sysevent.h"
#include "event_ids.h"
#include "filelog.h"
#include "sys_status.h"

#define MAX_BUFFER_CNT 10

static float battery_calculate_percentage(uint16_t voltage);

static const char* TAG = "adc";

/**
 * @brief Read the battery and pass it to sysevent
 *
 * @return int 0 on success, -1 on failure
 */
int read_battery_percentage(void) {
  int cal;
  uint16_t voltage;
  uint16_t battery_voltage[MAX_BUFFER_CNT];
  float bat_percent;
  char s_bat_percent[20];

  cal = battery_init();
  if (cal) {
    LOGI(TAG, "calibration completed");
  } else {
    LOGE(TAG, "calibration not support");
    return -1;
  }
  // Create a user task that uses the sensors.
  battery_read_on();
  vTaskDelay(500 / portTICK_PERIOD_MS);
  for (int i = 0; i < MAX_BUFFER_CNT; i++) {
    // Get the values and do something with them.
    battery_voltage[i] = read_battery_voltage(BATTERY_PORT, 2);
    LOGI(TAG, "cali data: %d mV", battery_voltage[i]);

    // Wait until 50 msec (cycle time) are over.
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
  battery_read_off();

  qsort(battery_voltage, MAX_BUFFER_CNT, sizeof(uint16_t), uint16_compare);
  voltage = uint16_average(&battery_voltage[1], MAX_BUFFER_CNT - 2);
  bat_percent = battery_calculate_percentage(voltage);
  LOGI(TAG, "battery percent: %.2f", bat_percent);
  FLOGI(TAG, "battery percent: %.2f", bat_percent);

  memset(s_bat_percent, 0, sizeof(s_bat_percent));
  snprintf(s_bat_percent, sizeof(s_bat_percent), "%.2f", bat_percent);

  if (bat_percent < 20) {
    set_low_battery(1);
  }
  sysevent_set(ADC_BATTERY_EVENT, (char*)s_bat_percent);

  return 0;
}

/**
 * @brief Calculate the battery value as a percentage
 *
 * @param voltage battery voltage
 *
 * @return percent value
 */
static float battery_calculate_percentage(uint16_t voltage) {
  float percent;

  if (voltage >= 4200) {
    percent = 100;
  } else if (voltage < 2400) {
    percent = 0;
  } else {
    percent = ((float)(voltage - 2400) / 1800) * 100;
  }

  return percent;
}
