#ifndef _MB_MASTER_IMPL_H_
#define _MB_MASTER_IMPL_H_

#include "mb_master_private.h"

#ifdef __cplusplus
extern "C" {
#endif

int mb_master_init_impl(int port, int baudrate, uart_params_t *uart);
int mb_master_register_characteristic_impl(mb_characteristic_info_t *mb_characteristic, int num_characteristic);
int mb_master_read_characteristic_impl(uint16_t cid, char *name, uint8_t *data, int *data_len);
int mb_master_write_characteristic_impl(uint16_t cid, char *name, uint8_t *data);
int mb_master_send_request_impl(uint8_t slave_id, uint8_t command, uint16_t reg_start, uint16_t size, void *data);
void mb_master_deinit_impl(void);

#ifdef __cplusplus
}
#endif

#endif /* _MB_MASTER_IMPL_H_ */
