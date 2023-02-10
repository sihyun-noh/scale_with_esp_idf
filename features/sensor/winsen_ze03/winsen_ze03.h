#ifndef _WINSEN_ZE03_H_
#define _WINSEN_ZE03_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  WINSEN_ZE03_OK = 0,
  WINSEN_ZE03_ERR_UART = -1,
  WINSEN_ZE03_ERR_CHECKSUM = -2,
  WINSEN_ZE03_ERR_STATUS = -3,
  WINSEN_ZE03_ERR_PARAMS = -4,
} winsen_ze03_err_code;

int winsen_ze03_init(void);

int winsen_ze03_read_manual(int *data);

uint8_t winsen_ze03_func_check_sum(uint8_t *i, uint8_t ln);

#ifdef __cplusplus
}
#endif

#endif /* _WINSEN_ZE03_H_ */
