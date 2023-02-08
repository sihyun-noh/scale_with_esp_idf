/*
  Morse.h - This library allows you to set and read the ZE03 Winsen Sensor module.
  Created by Fabian Gutierrez, March 12, 20017.
  MIT.
*/

#ifndef _WINSEN_ZE03_H_
#define _WINSEN_ZE03_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  WINSEN_ZE03_OK = 0,
  WINSEN_ZE03_ERR_UART = -1,
  WINSEN_ZE03_ERR_CRC = -2,
  WINSEN_ZE03_ERR_STATUS = -3,
  WINSEN_ZE03_ERR_PARAMS = -4,
} winsen_ze03_err_code;

int winsen_ze03_init(void);

void winsen_ze03_set_active(bool active);

float winsen_ze03_read_manual(void);

uint8_t func_check_sum(uint8_t *i, uint8_t ln);

#ifdef __cplusplus
}
#endif

#endif /* _WINSEN_ZE03_H_ */
