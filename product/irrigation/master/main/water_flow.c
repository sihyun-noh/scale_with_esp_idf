#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "log.h"

// static const char* TAG = "water_flow_task";

extern int get_water_flow_pulse_count(void);
extern void reset_water_flow_pulse_count(void);

int pulse_count;
int flow_liters;
float calibration_factor = 0.45;

static void water_flow_task(void* pvParameters) {
  for (;;) {
    pulse_count = get_water_flow_pulse_count();
    flow_liters = pulse_count / calibration_factor / 60;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    // LOGI(TAG, "flow_liters = %d L (%d)", flow_liters, pulse_count);
  }
}

int get_water_flow_liters(void) {
  return flow_liters;
}

void reset_water_flow_liters(void) {
  pulse_count = 0;
  flow_liters = 0;
  reset_water_flow_pulse_count();
}

void create_water_flow_task(void) {
  reset_water_flow_liters();

  xTaskCreate(water_flow_task, "water_flow_task", 2048, NULL, 10, NULL);
}
