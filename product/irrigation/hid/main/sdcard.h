#ifndef _SDCARD_H_
#define _SDCARD_H_

#ifdef __cplusplus
extern "C" {
#endif

void sdcard_init(void);
int sdcard_mount(void);
int sdcard_unmount(void);
int sdcard_get_status(void);

#ifdef __cplusplus
}
#endif

#endif  // _SDCARD_H_
