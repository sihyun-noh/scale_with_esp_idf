#ifndef _MB_MASTER_RTU_H_
#define _MB_MASTER_RTU_H_

#include "mb_master_private.h"

#ifdef __cplusplus
extern "C" {
#endif

void mb_set_uart_config(int rx, int tx, int rts, int cts);
int mb_master_init(int port, int baudrate);
int mb_master_register_characteristic(mb_characteristic_info_t *mb_characteristic, int num_characteristic);
int mb_master_read_characteristic(uint16_t cid, char *name, uint8_t *data, int *data_len);
int mb_master_write_characteristic(uint16_t cid, char *name, uint8_t *data);
int mb_master_send_request(uint8_t slave_id, uint8_t command, uint16_t reg_start, uint16_t size, void *data);
void mb_master_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* _MB_MASTER_RTU_H_ */
