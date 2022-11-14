#ifndef _SYS_STATUS_H_
#define _SYS_STATUS_H_

#define is_device_configured sys_stat_get_configured
#define set_device_configured sys_stat_set_configured

#define is_device_onboard sys_stat_get_onboard
#define set_device_onboard sys_stat_set_onboard

#define is_internet_available sys_stat_get_internet
#define set_internet_status sys_stat_set_internet

#define is_wifi_sta_available sys_stat_get_wifi_sta
#define set_wifi_sta_status sys_stat_set_wifi_sta

#define is_wifi_ap_available sys_stat_get_wifi_ap
#define set_wifi_ap_status sys_stat_set_wifi_ap

#define is_battery_model sys_stat_get_battery_model
#define set_battery_model sys_stat_set_battery_model

#define is_low_battery sys_stat_get_low_battery
#define set_low_battery sys_stat_set_low_battery

#define is_wifi_fail sys_stat_get_wifi_fail
#define set_wifi_fail sys_stat_set_wifi_fail

#define is_easy_setup_fail sys_stat_get_easy_setup_fail
#define set_easy_setup_fail sys_stat_set_easy_setup_fail

#define is_identification sys_stat_get_identification
#define set_identification sys_stat_set_identification

#define is_fwupdate sys_stat_get_fwupdate
#define set_fwupdate sys_stat_set_fwupdate

#define is_sdcard_fail sys_stat_get_sdcard_fail
#define set_sdcard_fail sys_stat_set_sdcard_fail

#define is_rs485_conn_fail sys_stat_get_rs485_conn_fail
#define set_rs485_conn_fail sys_stat_set_rs485_conn_fail

extern int sys_stat_get_configured(void);
extern void sys_stat_set_configured(uint8_t status);

extern int sys_stat_get_onboard(void);
extern void sys_stat_set_onboard(uint8_t status);

extern int sys_stat_get_fwupdate(void);
extern void sys_stat_set_fwupdate(uint8_t status);

extern int sys_stat_get_internet(void);
extern void sys_stat_set_internet(uint8_t status);

extern int sys_stat_get_wifi_ap(void);
extern void sys_stat_set_wifi_ap(uint8_t status);

extern int sys_stat_get_wifi_sta(void);
extern void sys_stat_set_wifi_sta(uint8_t status);

extern int sys_stat_get_battery_model(void);
extern void sys_stat_set_battery_model(uint8_t status);

extern int sys_stat_get_low_battery(void);
extern void sys_stat_set_low_battery(uint8_t status);

extern int sys_stat_get_wifi_fail(void);
extern void sys_stat_set_wifi_fail(uint8_t status);

extern int sys_stat_get_easy_setup_fail(void);
extern void sys_stat_set_easy_setup_fail(uint8_t status);

extern int sys_stat_get_identification(void);
extern void sys_stat_set_identification(uint8_t status);

extern int sys_stat_get_sdcard_fail(void);
extern void sys_stat_set_sdcard_fail(uint8_t status);

extern int sys_stat_get_rs485_conn_fail(void);
extern void sys_stat_set_rs485_conn_fail(uint8_t status);

extern int sys_stat_init(void);

#endif /* _SYS_STATUS_H_ */
