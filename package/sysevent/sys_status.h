#ifndef _SYS_STATUS_H_
#define _SYS_STATUS_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO: Please add the service event and hardware event that you want to monitor in the code below.

/* Service Status */
#define STATUS_CONFIGURED (1 << 0)         /* Device is configured after finishing easy setup progress */
#define STATUS_ONBOARD (1 << 1)            /* Device onboard on the network */
#define STATUS_FWUPDATE (1 << 2)           /* OTA FW Update */
#define STATUS_MQTT_CONNECTED (1 << 3)     /* mqtt client is connected */
#define STATUS_MQTT_SUBSCRIBED (1 << 4)    /* mqtt topic is subscribed */
#define STATUS_MQTT_PUBLISHED (1 << 5)     /* mqtt topic/payload is published */
#define STATUS_MQTT_INIT_FINISHED (1 << 6) /* mqtt client is initialized */

/* Hardware Status */
#define STATUS_RESET (1 << 0)          /* HW reset (factory reset) */
#define STATUS_INTERNET (1 << 1)       /* Indicator event when internet is available */
#define STATUS_WIFI_AP (1 << 2)        /* Use event when WiFi AP mode comes up */
#define STATUS_WIFI_STA (1 << 3)       /* Use event when WiFi Sta mode comes up */
#define STATUS_BATTERY (1 << 4)        /* Use event to check battery model */
#define STATUS_WIFI_SCANNING (1 << 5)  /* wifi scanning */
#define STATUS_WIFI_SCAN_DONE (1 << 6) /* wifi scanning done */

/* LED Status */
#define STATUS_OK (1 << 0)              /* Normal operation */
#define STATUS_LOW_BATTERY (1 << 1)     /* Battery is below 20% */
#define STATUS_WIFI_FAIL (1 << 2)       /* WiFi is not connected */
#define STATUS_EASY_SETUP_FAIL (1 << 3) /* Easy setup is not completed */
#define STATUS_IDENTIFICATION (1 << 4)  /* Identification is running */

/* Actuator Status */
#define STATUS_OPEN (1 << 0)  /* Open  */
#define STATUS_ERROR (1 << 1) /* Error */

/* Irrigation Status */
#define STATUS_TIME_SYNC (1 << 0)        /* Time sync */
#define STATUS_BATTERY_LEVEL (1 << 1)    /* Battery level */
#define STATUS_SET_CONFIG (1 << 2)       /* Set Configuration */
#define STATUS_START_IRRIGATION (1 << 3) /* Start Irrigation */
#define STATUS_STOP_IRRIGATION (1 << 4)  /* Stop Irrigation */

/* Data logger Status */
typedef enum {
  USB_COPYING = 0,
  USB_COPY_FAIL,
  USB_COPY_SUCCESS,
} usb_stat_t;
/* V1 */
#define STATUS_SDCARD_FAIL (1 << 0)     /* Data logger confirm SD card insert */
#define STATUS_RS485_CONN_FAIL (1 << 1) /* Data logger comfirm sensor connect */
/* V2 */
#define STATUS_USB_COPYING (1 << 2)      /* Data logger logfile copying */
#define STATUS_USB_COPY_FAIL (1 << 3)    /* Data logger logfile copy fail */
#define STATUS_USB_COPY_SUCCESS (1 << 4) /* Data logger logfile copy success */
#define STATUS_USB_DISCONN (1 << 5)      /* Data logger notify USB disconnect */
#define STATUS_LOG_FILE_WRITE (1 << 6)   /* Data logger notify file write */

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

#define is_wifi_scanning sys_stat_get_wifi_scanning
#define set_wifi_scanning sys_stat_set_wifi_scanning

#define is_wifi_scan_done sys_stat_get_wifi_scan_done
#define set_wifi_scan_done sys_stat_set_wifi_scan_done

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

#define is_usb_copying(x) sys_stat_get_usb_copying(x)
#define set_usb_copying(x, y) sys_stat_set_usb_copying(x, y)

#define is_usb_disconnect_notify sys_stat_get_usb_disconnect_notify
#define set_usb_disconnect_notify sys_stat_set_usb_disconnect_notify

#define is_log_file_write_flag sys_stat_get_file_write_flag
#define set_log_file_write_flag sys_stat_set_file_write_flag

#define is_mqtt_connected sys_stat_get_mqtt_connected
#define set_mqtt_connected sys_stat_set_mqtt_connected

#define is_mqtt_subscribed sys_stat_get_mqtt_subscribed
#define set_mqtt_subscribed sys_stat_set_mqtt_subscribed

#define is_mqtt_published sys_stat_get_mqtt_published
#define set_mqtt_published sys_stat_set_mqtt_published

#define is_mqtt_init_finished sys_stat_get_mqtt_init_finished
#define set_mqtt_init_finished sys_stat_set_mqtt_init_finished

#define wait_for_sw_event sys_stat_wait_for_swevent
#define wait_for_hw_event sys_stat_wait_for_hwevent

#define is_actuator_open sys_stat_get_actuator_open
#define set_actuator_open sys_stat_set_actuator_open

#define is_actuator_err sys_stat_get_actuator_err
#define set_actuator_err sys_stat_set_actuator_err

#define is_time_sync sys_stat_get_time_sync
#define set_time_sync sys_stat_set_time_sync

#define is_battery_level sys_stat_get_battery_level
#define set_battery_level sys_stat_set_battery_level

#define is_start_irrigation sys_stat_get_start_irrigation
#define set_start_irrigation sys_stat_set_start_irrigation

#define is_stop_irrigation sys_stat_get_stop_irrigation
#define set_stop_irrigation sys_stat_set_stop_irrigation

int sys_stat_get_configured(void);
void sys_stat_set_configured(uint8_t status);

int sys_stat_get_onboard(void);
void sys_stat_set_onboard(uint8_t status);

int sys_stat_get_fwupdate(void);
void sys_stat_set_fwupdate(uint8_t status);

int sys_stat_get_mqtt_connected(void);
void sys_stat_set_mqtt_connected(uint8_t status);

int sys_stat_get_mqtt_published(void);
void sys_stat_set_mqtt_published(uint8_t status);

int sys_stat_get_mqtt_subscribed(void);
void sys_stat_set_mqtt_subscribed(uint8_t status);

int sys_stat_get_mqtt_init_finished(void);
void sys_stat_set_mqtt_init_finished(uint8_t status);

int sys_stat_get_internet(void);
void sys_stat_set_internet(uint8_t status);

int sys_stat_get_wifi_ap(void);
void sys_stat_set_wifi_ap(uint8_t status);

int sys_stat_get_wifi_sta(void);
void sys_stat_set_wifi_sta(uint8_t status);

int sys_stat_get_battery_model(void);
void sys_stat_set_battery_model(uint8_t status);

int sys_stat_get_wifi_scanning(void);
void sys_stat_set_wifi_scanning(uint8_t status);

int sys_stat_get_wifi_scan_done(void);
void sys_stat_set_wifi_scan_done(uint8_t status);

int sys_stat_get_low_battery(void);
void sys_stat_set_low_battery(uint8_t status);

int sys_stat_get_wifi_fail(void);
void sys_stat_set_wifi_fail(uint8_t status);

int sys_stat_get_easy_setup_fail(void);
void sys_stat_set_easy_setup_fail(uint8_t status);

int sys_stat_get_identification(void);
void sys_stat_set_identification(uint8_t status);

int sys_stat_get_sdcard_fail(void);
void sys_stat_set_sdcard_fail(uint8_t status);

int sys_stat_get_rs485_conn_fail(void);
void sys_stat_set_rs485_conn_fail(uint8_t status);

int sys_stat_init(void);

bool sys_stat_wait_for_swevent(int event, int timeout_ms);
bool sys_stat_wait_for_hwevent(int event, int timeout_ms);

int sys_stat_get_actuator_open(void);
void sys_stat_set_actuator_open(uint8_t status);

int sys_stat_get_actuator_err(void);
void sys_stat_set_actuator_err(uint8_t status);

int sys_stat_get_time_sync(void);
void sys_stat_set_time_sync(uint8_t status);

int sys_stat_get_battery_level(void);
void sys_stat_set_battery_level(uint8_t status);

int sys_stat_get_start_irrigation(void);
void sys_stat_set_start_irrigation(uint8_t status);

int sys_stat_get_stop_irrigation(void);
void sys_stat_set_stop_irrigation(uint8_t status);

int sys_stat_get_usb_copying(usb_stat_t usb_status);
void sys_stat_set_usb_copying(usb_stat_t usb_status, uint8_t status);

int sys_stat_get_usb_disconnect_notify(void);
void sys_stat_set_usb_disconnect_notify(uint8_t status);

int sys_stat_get_file_write_flag(void);
void sys_stat_set_file_write_flag(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_STATUS_H_ */
