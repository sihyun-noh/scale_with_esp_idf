#ifndef _UART_HAL_H_
#define _UART_HAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define HW_FLOWCTRL_DISABLE 0x0  // disable HW Flow Control
#define HW_FLOWCTRL_RTS 0x1      // use only RTS PIN for HW Flow Control
#define HW_FLOWCTRL_CTS 0x2      // use only CTS PIN for HW Flow Control
#define HW_FLOWCTRL_CTS_RTS 0x3  // use both CTS and RTS PIN for HW Flow Control

typedef enum {
  HAL_UART_NO_ERR = 0,
  HAL_UART_INIT_ERR = -1,
  HAL_UART_READ_ERR = -2,
  HAL_UART_WRITE_ERR = -3,
  HAL_UART_INVALID_ARGS = -4,
  HAL_UART_INVALID_IFACE = -5
} uart_hal_err_t;

int uart_hal_initialize(int dev, uint32_t baudrate, int8_t rxPin, int8_t txPin, uint16_t rx_buffer_size,
                        uint16_t tx_buffer_size);

void uart_hal_free(int dev);

uint32_t uart_available(int dev);

int uart_hal_read(int dev, void *buf, uint32_t length);

int uart_hal_write(int dev, const uint8_t *data, size_t len);

void uart_hal_flush(int dev);

void uart_hal_flush_tx_only(int dev, bool tx_only);

int uart_hal_baudrate(int dev, uint32_t baud_rate);

#ifdef __cplusplus
}
#endif

#endif /* _UART_HAL_H_ */
