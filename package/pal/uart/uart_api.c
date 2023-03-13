#include "uart_api.h"
#include "esp32/uart_pal.h"

int uart_initialize(uint8_t uart_nr, uint32_t baudrate, int8_t rxPin, int8_t txPin, uint16_t rx_buffer_size, uint16_t tx_buffer_size) {
  return uart_hal_initialize(uart_nr, baudrate, rxPin, txPin, rx_buffer_size, tx_buffer_size);
}

int uart_read_data(int dev,uint8_t *data, size_t len) {
  return uart_hal_read(dev, data, len);
}

int uart_write_data(int dev, const uint8_t *data, size_t len) {
  return uart_hal_write(dev, data, len);
}

int uart_set_baud(int dev, uint32_t baud_rate) {
  return uart_hal_baudrate(dev, baud_rate);
}
