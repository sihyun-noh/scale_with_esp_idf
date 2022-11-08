#ifndef _MB_MASTER_PRIVATE_H_
#define _MB_MASTER_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  MB_HOLDING_REG = 0x00,
  MB_INPUT_REG,
  MB_COIL_REG,
  MB_DISCRETE_REG,
} mb_register_t;

typedef enum {
  DATA_U8 = 0x00,
  DATA_U16,
  DATA_U32,
  DATA_FLOAT,
  DATA_BINARY,
} mb_data_t;

typedef struct {
  int rx;
  int tx;
  int rts;
  int cts;
} uart_params_t;

typedef struct {
  uint16_t cid;
  char *name;
  char *units;
  mb_data_t data_type;
  uint16_t data_len;
  uint8_t slave_id;
  mb_register_t register_type;
  uint16_t reg_addr;
  uint16_t read_len;
} mb_characteristic_info_t;

#ifdef __cplusplus
}
#endif

#endif /* _MB_MASTER_PRIVATE_H_ */
