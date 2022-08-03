#include "syslog.h"

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>

#define TAG "sys_status"
#define dbg(format, args...) LOGI(TAG, format, ##args)
#define dbgw(format, args...) LOGW(TAG, format, ##args)
#define dbge(format, args...) LOGE(TAG, format, ##args)

// TODO: Please add the service event and hardware event that you want to monitor in the code below.

/* Service Status */
#define STATUS_CONFIGURED (1 << 0) /* Device is configured after finishing easy setup progress */

/* Hardware Status */
#define STATUS_RESET (1 << 0)    /* HW reset (factory reset) */
#define STATUS_INTERNET (1 << 1) /* Indicator event when internet is available */
#define STATUS_WIFI_AP (1 << 2)  /* Use event when WiFi AP mode comes up */
#define STATUS_WIFI_STA (1 << 3) /* Use event when WiFi Sta mode comes up */

/* LED Status */
#define STATUS_OK (1 << 0)              /* Normal operation */
#define STATUS_LOW_BATTERY (1 << 1)     /* Battery is below 20% */
#define STATUS_WIFI_FAIL (1 << 2)       /* WiFi is not connected */
#define STATUS_EASY_SETUP_FAIL (1 << 3) /* Easy setup is not completed */
#define STATUS_IDENTIFICATION (1 << 4)  /* Identification is running */

static EventGroupHandle_t hw_status_events;
static EventGroupHandle_t sw_status_events;
static EventGroupHandle_t led_status_events;

/*----------------------------------------------------------------*/
/*                      Service Status                            */
/*----------------------------------------------------------------*/

int sys_stat_get_configured(void) {
  EventBits_t bits = xEventGroupGetBits(sw_status_events);
  return ((bits & STATUS_CONFIGURED) == STATUS_CONFIGURED);
}

void sys_stat_set_configured(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(sw_status_events, STATUS_CONFIGURED);
  } else {
    xEventGroupClearBits(sw_status_events, STATUS_CONFIGURED);
  }
}

/*----------------------------------------------------------------*/
/*                     Hardware Status                            */
/*----------------------------------------------------------------*/

int sys_stat_get_internet(void) {
  EventBits_t bits = xEventGroupGetBits(hw_status_events);
  return ((bits & STATUS_INTERNET) == STATUS_INTERNET);
}

void sys_stat_set_internet(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(hw_status_events, STATUS_INTERNET);
  } else {
    xEventGroupClearBits(hw_status_events, STATUS_INTERNET);
  }
}

int sys_stat_get_wifi_sta(void) {
  EventBits_t bits = xEventGroupGetBits(hw_status_events);
  return ((bits & STATUS_WIFI_STA) == STATUS_WIFI_STA);
}

void sys_stat_set_wifi_sta(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(hw_status_events, STATUS_WIFI_STA);
  } else {
    xEventGroupClearBits(hw_status_events, STATUS_WIFI_STA);
  }
}

int sys_stat_get_wifi_ap(void) {
  EventBits_t bits = xEventGroupGetBits(hw_status_events);
  return ((bits & STATUS_WIFI_AP) == STATUS_WIFI_AP);
}

void sys_stat_set_wifi_ap(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(hw_status_events, STATUS_WIFI_AP);
  } else {
    xEventGroupClearBits(hw_status_events, STATUS_WIFI_AP);
  }
}

/*----------------------------------------------------------------*/
/*                     LED Status                                 */
/*----------------------------------------------------------------*/

int sys_stat_get_low_battery(void) {
  EventBits_t bits = xEventGroupGetBits(led_status_events);
  return ((bits & STATUS_LOW_BATTERY) == STATUS_LOW_BATTERY);
}

void sys_stat_set_low_battery(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(led_status_events, STATUS_LOW_BATTERY);
  } else {
    xEventGroupClearBits(led_status_events, STATUS_LOW_BATTERY);
  }
}

int sys_stat_get_wifi_fail(void) {
  EventBits_t bits = xEventGroupGetBits(led_status_events);
  return ((bits & STATUS_WIFI_FAIL) == STATUS_WIFI_FAIL);
}

void sys_stat_set_wifi_fail(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(led_status_events, STATUS_WIFI_FAIL);
  } else {
    xEventGroupClearBits(led_status_events, STATUS_WIFI_FAIL);
  }
}

int sys_stat_get_easy_setup_fail(void) {
  EventBits_t bits = xEventGroupGetBits(led_status_events);
  return ((bits & STATUS_EASY_SETUP_FAIL) == STATUS_EASY_SETUP_FAIL);
}

void sys_stat_set_easy_setup_fail(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(led_status_events, STATUS_EASY_SETUP_FAIL);
  } else {
    xEventGroupClearBits(led_status_events, STATUS_EASY_SETUP_FAIL);
  }
}

int sys_stat_get_identification(void) {
  EventBits_t bits = xEventGroupGetBits(led_status_events);
  return ((bits & STATUS_IDENTIFICATION) == STATUS_IDENTIFICATION);
}

void sys_stat_set_identification(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(led_status_events, STATUS_IDENTIFICATION);
  } else {
    xEventGroupClearBits(led_status_events, STATUS_IDENTIFICATION);
  }
}

int sys_stat_init(void) {
  sw_status_events = xEventGroupCreate();
  if (sw_status_events == NULL) {
    dbge("Failed to create event group for service status");
    return -1;
  }

  hw_status_events = xEventGroupCreate();
  if (hw_status_events == NULL) {
    dbge("Failed to create event group for hardware status");
    return -1;
  }

  led_status_events = xEventGroupCreate();
  if (led_status_events == NULL) {
    dbge("Failed to create event group for LED status");
    return -1;
  }

  return 0;
}
