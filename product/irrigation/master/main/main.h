#ifndef _MAIN_H_
#define _MAIN_H_

typedef enum {
  SYSINIT_OK,
  ERR_NVS_FLASH,
  ERR_WIFI_INIT,
  ERR_SYSCFG_INIT,
  ERR_SYSCFG_OPEN,
  ERR_SYSEVENT_CREATE,
  ERR_SYS_STATUS_INIT,
  ERR_SPIFFS_INIT,
} err_sysinit_t;

typedef enum {
  CHECK_OK,
  ERR_BATTERY_READ,
} err_system_t;

typedef enum {
  START_MODE = 0,
  CONTROL_MODE,
  MONITOR_MODE,
  DEEP_SLEEP_MODE,
} operation_mode_t;

#ifdef __cplusplus
extern "C" {
#endif

int sleep_timer_wakeup(uint64_t wakeup_time_sec);

#ifdef __cplusplus
}
#endif

#endif /* _MAIN_H_ */
