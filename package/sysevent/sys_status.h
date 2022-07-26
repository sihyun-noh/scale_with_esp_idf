#ifndef _SYS_STATUS_H_
#define _SYS_STATUS_H_

#define is_device_configured sys_stat_get_configured
#define set_device_configured sys_stat_set_configured

#define is_internet_available sys_stat_get_internet
#define set_internet_status sys_stat_set_internet

#define is_wifi_sta_available sys_stat_get_wifi_sta
#define set_wifi_sta_status sys_stat_set_wifi_sta

#define is_wifi_ap_available sys_stat_get_wifi_ap
#define set_wifi_ap_status sys_stat_set_wifi_ap

extern int sys_stat_get_configured(void);
extern void sys_stat_set_configured(uint8_t status);

extern int sys_stat_get_internet(void);
extern void sys_stat_set_internet(uint8_t status);

extern int sys_stat_get_wifi_ap(void);
extern void sys_stat_set_wifi_ap(uint8_t status);

extern int sys_stat_get_wifi_sta(void);
extern void sys_stat_set_wifi_sta(uint8_t status);

extern int sys_stat_init(void);

#endif /* _SYS_STATUS_H_ */
