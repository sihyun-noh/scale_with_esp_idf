#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "config.h"
#include "log.h"
#include "actuator.h"

#define GPIO_INPUT_PIN_SEL ((1ULL << FUNC_KEY1) | (1ULL << FUNC_KEY2) | (1ULL << WATER_FLOW))
#define ESP_INTR_FLAG_DEFAULT 0

extern void pump_on(void);
extern void pump_off(void);

int water_flow_pulse_count;

// static const char* TAG = "key_task";

static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg) {
  uint32_t gpio_num = (uint32_t)arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void input_task(void* arg) {
  uint32_t io_num;
  for (;;) {
    if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
      // LOGI(TAG, "GPIO[%" PRIu32 "] intr, val: %d\n", io_num, gpio_get_level(io_num));
      if (io_num == FUNC_KEY1)
        pump_on();
      else if (io_num == FUNC_KEY2)
        pump_off();
      else if (io_num == WATER_FLOW)
        water_flow_pulse_count++;
    }
  }
}

int get_water_flow_pulse_count(void) {
  return water_flow_pulse_count;
}

void reset_water_flow_pulse_count(void) {
  water_flow_pulse_count = 0;
}

void create_input_task(void) {
  water_flow_pulse_count = 0;

  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_NEGEDGE;
  io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = 1;
  gpio_config(&io_conf);

  gpio_set_intr_type(WATER_FLOW, GPIO_INTR_POSEDGE);

  gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
  xTaskCreate(input_task, "input_task", 2048, NULL, 10, NULL);

  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  gpio_isr_handler_add(FUNC_KEY1, gpio_isr_handler, (void*)FUNC_KEY1);
  gpio_isr_handler_add(FUNC_KEY2, gpio_isr_handler, (void*)FUNC_KEY2);
  gpio_isr_handler_add(WATER_FLOW, gpio_isr_handler, (void*)WATER_FLOW);
}
