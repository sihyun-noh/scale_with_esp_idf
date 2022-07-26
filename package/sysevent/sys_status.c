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

static EventGroupHandle_t hw_status_events;
static EventGroupHandle_t sw_status_events;

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

  return 0;
}
