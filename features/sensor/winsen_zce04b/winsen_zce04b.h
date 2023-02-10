#ifndef _WINSEN_ZCE04B_H_
#define _WINSEN_ZCE04B_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  WINSEN_ZCE04B_OK = 0,
  WINSEN_ZCE04B_ERR_UART = -1,
  WINSEN_ZCE04B_ERR_CHECKSUM = -2,
  WINSEN_ZCE04B_ERR_STATUS = -3,
  WINSEN_ZCE04B_ERR_PARAMS = -4,
} winsen_zce04b_err_code;

typedef struct {
  int co;
  int h2s;
  float o2;
  int ch4;
} zce04b_data_t;

int winsen_zce04b_init(void);

int winsen_zce04b_read_manual(zce04b_data_t *data);

uint8_t winsen_zce04b_func_check_sum(uint8_t *i, uint8_t ln);

#ifdef __cplusplus
}
#endif

#endif /* _WINSEN_ZCE04B_H_ */
