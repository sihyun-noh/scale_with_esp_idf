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
  ERR_MONITORING_INIT,
  ERR_SPIFFS_INIT,
} err_sysinit_t;

typedef enum {
  CHECK_OK,
  SENSOR_NOT_PUB,
  ERR_SENSOR_READ,
  ERR_BATTERY_READ,
} err_system_t;

typedef enum {
  SENSOR_INIT_MODE = 0,
  SENSOR_READ_MODE,
  SENSOR_PUB_MODE,
  SLEEP_MODE
} operation_mode_t;

#endif /* _MAIN_H_ */