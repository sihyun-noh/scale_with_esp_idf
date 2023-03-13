#ifndef _UART_API_H_
#define _UART_API_H_

#include "stddef.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

int uart_initialize(uint8_t uart_nr, uint32_t baudrate, int8_t rxPin, int8_t txPin, uint16_t rx_buffer_size, uint16_t tx_buffer_size);

int uart_read_data(int dev,uint8_t *data, size_t len);

int uart_write_data(int dev, const uint8_t *data, size_t len);

int uart_set_baud(int dev, uint32_t baud_rate);

#ifdef __cplusplus
}
#endif

#endif /* _UART_API_H_ */
