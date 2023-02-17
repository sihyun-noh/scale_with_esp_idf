#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
  SYSINIT_OK,
  ERR_NVS_FLASH,
  ERR_SYSCFG_INIT,
  ERR_SYSCFG_OPEN,
  ERR_SYSEVENT_CREATE,
  ERR_SYS_STATUS_INIT,
  ERR_FILESYSTEM_INIT,
} err_sysinit_t;

typedef enum {
  CHECK_OK,
  SENSOR_NOT_PUB,
  ERR_SENSOR_READ,
} err_system_t;

#define ROOT_PATH "/storage"
#define PART_NAME "storage"
#define DIR_PATH "/storage/data"
#define FILE_NAME "/sensor"

typedef enum { SENSOR_INIT_MODE = 0, SENSOR_READ_MODE, SLEEP_MODE } operation_mode_t;

#ifdef __cplusplus
}
#endif
#endif /* _MAIN_H_ */
