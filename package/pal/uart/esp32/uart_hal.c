#include <stdio.h>
#include "freertos/portmacro.h"

#include "driver/uart.h"
#include "hal/uart_ll.h"
#include "uart_hal.h"

typedef struct uart_hal {
  uint8_t num;
  bool has_peek;
  uint8_t peek_byte;
  SemaphoreHandle_t mutex;
  QueueHandle_t uart_event_queue;
} uart_hal_t;

static uart_hal_t uart_hal_data[3] = {
  { 0, false, 0, NULL, NULL },
  { 1, false, 0, NULL, NULL },
  { 2, false, 0, NULL, NULL },
};

static uart_hal_t* get_uart_interface(int dev) {
  return &uart_hal_data[dev];
}

void uart_hal_lock(int dev) {
  uart_hal_t* hal = get_uart_interface(dev);
  if (hal) {
    xSemaphoreTake(hal->mutex, portMAX_DELAY);
  }
}

void uart_hal_unlock(int dev) {
  uart_hal_t* hal = get_uart_interface(dev);
  if (hal) {
    xSemaphoreGive(hal->mutex);
  }
}

int uart_hal_initialize(int dev, uint32_t baudrate, int8_t rxPin, int8_t txPin, uint16_t rx_buffer_size,
                        uint16_t tx_buffer_size) {
  esp_err_t rc = ESP_FAIL;

  if (dev >= 3) {
    return HAL_UART_INIT_ERR;
  }

  uart_hal_t* uart = &uart_hal_data[dev];

  if (uart_is_driver_installed(dev)) {
    uart_hal_free(dev);
  }

  if (uart->mutex == NULL) {
    uart->mutex = xSemaphoreCreateMutex();
    if (uart->mutex == NULL) {
      return HAL_UART_INIT_ERR;
    }
  }

  uart_hal_lock(dev);

  uart_config_t uart_config;
  uart_config.baud_rate = baudrate;
  uart_config.data_bits = UART_DATA_8_BITS;
  uart_config.parity = UART_PARITY_DISABLE;
  uart_config.stop_bits = UART_STOP_BITS_1;
  uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
  uart_config.source_clk = UART_SCLK_APB;

  rc = uart_driver_install(dev, rx_buffer_size, tx_buffer_size, 20, &(uart->uart_event_queue), 0);
  rc = uart_param_config(dev, &uart_config);
  rc = uart_set_pin(dev, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  uart_hal_unlock(dev);

  uart_hal_flush(dev);

  if (ESP_OK == rc) {
    return HAL_UART_NO_ERR;
  }

  return HAL_UART_INIT_ERR;
}

void uart_hal_free(int dev) {
  uart_hal_t* uart = get_uart_interface(dev);
  if (uart == NULL) {
    return;
  }

  uart_hal_lock(dev);
  uart_driver_delete(dev);
  uart_hal_unlock(dev);
}

uint32_t uart_available(int dev) {
  uart_hal_t* uart = get_uart_interface(dev);
  if (uart == NULL) {
    return 0;
  }

  uart_hal_lock(dev);
  size_t available;
  uart_get_buffered_data_len(dev, &available);
  if (uart->has_peek)
    available++;
  uart_hal_unlock(dev);
  return available;
}

int uart_hal_read(int dev, void* buf, uint32_t length) {
  uart_hal_t* uart = get_uart_interface(dev);
  if (uart == NULL) {
    return 0;
  }

  uart_hal_lock(dev);

  int len = uart_read_bytes(dev, buf, length, 20 / portTICK_PERIOD_MS);

  uart_hal_unlock(dev);

  return len;
}

int uart_hal_write(int dev, const uint8_t* data, size_t len) {
  uart_hal_t* uart = get_uart_interface(dev);
  if (uart == NULL || data == NULL || !len) {
    return HAL_UART_WRITE_ERR;
  }

  uart_hal_lock(dev);
  esp_err_t rc = uart_write_bytes(dev, data, len);
  uart_hal_unlock(dev);

  if (ESP_OK != rc) {
    return HAL_UART_WRITE_ERR;
  }

  return HAL_UART_NO_ERR;
}

void uart_hal_flush(int dev) {
  uart_hal_flush_tx_only(dev, true);
}

void uart_hal_flush_tx_only(int dev, bool tx_only) {
  uart_hal_t* uart = get_uart_interface(dev);
  if (uart == NULL) {
    return;
  }

  uart_hal_lock(dev);
  while (!uart_ll_is_tx_idle(UART_LL_GET_HW(dev)))
    ;

  if (!tx_only) {
    ESP_ERROR_CHECK(uart_flush_input(dev));
  }
  uart_hal_unlock(dev);
}
