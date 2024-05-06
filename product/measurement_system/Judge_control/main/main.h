#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  SYSINIT_OK,
  ERR_NVS_FLASH,
  ERR_WIFI_INIT,
  ERR_SYSCFG_INIT,
  ERR_SYSCFG_OPEN,
  ERR_SYSEVENT_CREATE,
  ERR_SYS_STATUS_INIT,
  ERR_MONITORING_INIT,
  ERR_SPIFFS_INIT,
} err_sysinit_t;

typedef enum {
  CHECK_OK,
  ERR_BATTERY_READ,
} err_system_t;

typedef enum {
  CH_1_SET,
  CH_2_SET,
} mux_ctrl_t;

typedef enum { INIT_MODE = 0, CONNECT_MODE, MONITOR_MODE, DEEPSLEEP_MODE } operation_mode_t;

void SET_CH1_PIN(void);
void SET_CH2_PIN(void);
void SET_MUX_CONTROL(mux_ctrl_t ch);
void led_1_ctrl(uint8_t ctrl);
void led_2_ctrl(uint8_t ctrl);
void led_3_ctrl(uint8_t ctrl);

extern char msc_mode_check[5];
extern char indicator_model_buf[20];

#ifdef __cplusplus
}
#endif

#endif /* _MAIN_H_ */
