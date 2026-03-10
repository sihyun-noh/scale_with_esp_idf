
// board_i2c.h
#ifndef _BOARD_I2C_H_
#define _BOARD_I2C_H_
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// 공용 I2C 초기화 함수
esp_err_t board_i2c_init(void);

#ifdef __cplusplus
}
#endif
#endif
