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
  SENSOR_NOT_PUB,
  ERR_SENSOR_READ,
  ERR_BATTERY_READ,
} err_system_t;

typedef enum {
  SEN1 = 0,
  SEN2,
  SEN3,
} sen_set_t;

#if (CONFIG_LITTLEFS_PACKAGE)
#define SEN_DATA_PATH_1 "/storage/Sen1"
#define SEN_DATA_PATH_2 "/storage/Sen2"
#define SEN_DATA_PATH_3 "/storage/Sen3"
#else
#define SEN_DATA_PATH_1 "/spiffs/Sen1"
#define SEN_DATA_PATH_2 "/spiffs/Sen2"
#define SEN_DATA_PATH_3 "/spiffs/Sen3"
#endif
typedef enum { SENSOR_INIT_MODE = 0, SENSOR_READ_MODE, SENSOR_PUB_MODE, SLEEP_MODE } operation_mode_t;

#ifdef __cplusplus
}
#endif
#endif /* _MAIN_H_ */
