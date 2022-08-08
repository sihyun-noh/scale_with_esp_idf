#include <stdio.h>
#include <string.h>

#include "mb_master_rtu.h"
#include "esp32/mb_master_impl.h"

static uart_params_t uart_config;

static uart_params_t *get_uart_config(void) {
  return &uart_config;
}

void mb_set_uart_config(int rx, int tx, int rts, int cts) {
  uart_config.rx = rx;
  uart_config.tx = tx;
  uart_config.rts = rts;
  uart_config.cts = cts;
}

int mb_master_init(int port, int baudrate) {
  return mb_master_init_impl(port, baudrate, get_uart_config());
}

int mb_master_register_characteristic(mb_characteristic_info_t *mb_characteristic, int num_characteristic) {
  return mb_master_register_characteristic_impl(mb_characteristic, num_characteristic);
}

int mb_master_read_characteristic(uint16_t cid, char *name, uint8_t *data, int *data_len) {
  return mb_master_read_characteristic_impl(cid, name, data, data_len);
}

int mb_master_write_characteristic(uint16_t cid, char *name, uint8_t *data) {
  return mb_master_write_characteristic_impl(cid, name, data);
}

int mb_master_send_request(uint8_t slave_id, uint8_t command, uint16_t reg_start, uint16_t size, void *data) {
  return mb_master_send_request_impl(slave_id, command, reg_start, size, data);
}

void mb_master_deinit(void) {
  mb_master_deinit_impl();
}
