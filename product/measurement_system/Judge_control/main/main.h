#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SYSCFG_N_USB_MODE "usb_mode"
#define SYSCFG_S_USB_MODE 5
#define SYSCFG_I_USB_MODE CFG_DATA

#define SYSCFG_N_INDICATOR_SET "indicator_set"
#define SYSCFG_S_INDICATOR_SET 20
#define SYSCFG_I_INDICATOR_SET CFG_DATA

#define SYSCFG_N_SPEAKER "speaker_set"
#define SYSCFG_S_SPEAKER 5
#define SYSCFG_I_SPEAKER CFG_DATA

#define SYSCFG_N_PRINTER "printer_set"
#define SYSCFG_S_PRINTER 5
#define SYSCFG_I_PRINTER CFG_DATA

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

extern char usb_mode[5];
extern char speaker_set[5];
extern char printer_set[5];
extern char indicator_set[20];

#ifdef __cplusplus
}
#endif

#endif /* _MAIN_H_ */
