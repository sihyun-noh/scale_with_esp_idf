#include "syslog.h"

#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>

#include "sys_status.h"

#define TAG "sys_status"
#define dbg(format, args...) LOGI(TAG, format, ##args)
#define dbgw(format, args...) LOGW(TAG, format, ##args)
#define dbge(format, args...) LOGE(TAG, format, ##args)

static EventGroupHandle_t hw_status_events;
static EventGroupHandle_t sw_status_events;
static EventGroupHandle_t led_status_events;
static EventGroupHandle_t actuator_status_events;
static EventGroupHandle_t irrigation_status_events;
static EventGroupHandle_t data_logger_status_events;

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

int sys_stat_get_onboard(void) {
  EventBits_t bits = xEventGroupGetBits(sw_status_events);
  return ((bits & STATUS_ONBOARD) == STATUS_ONBOARD);
}

void sys_stat_set_onboard(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(sw_status_events, STATUS_ONBOARD);
  } else {
    xEventGroupClearBits(sw_status_events, STATUS_ONBOARD);
  }
}

void sys_stat_set_fwupdate(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(sw_status_events, STATUS_FWUPDATE);
  } else {
    xEventGroupClearBits(sw_status_events, STATUS_FWUPDATE);
  }
}

int sys_stat_get_fwupdate(void) {
  EventBits_t bits = xEventGroupGetBits(sw_status_events);
  return ((bits & STATUS_FWUPDATE) == STATUS_FWUPDATE);
}

void sys_stat_set_mqtt_connected(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(sw_status_events, STATUS_MQTT_CONNECTED);
  } else {
    xEventGroupClearBits(sw_status_events, STATUS_MQTT_CONNECTED);
  }
}

int sys_stat_get_mqtt_connected(void) {
  EventBits_t bits = xEventGroupGetBits(sw_status_events);
  return ((bits & STATUS_MQTT_CONNECTED) == STATUS_MQTT_CONNECTED);
}

void sys_stat_set_mqtt_published(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(sw_status_events, STATUS_MQTT_PUBLISHED);
  } else {
    xEventGroupClearBits(sw_status_events, STATUS_MQTT_PUBLISHED);
  }
}

int sys_stat_get_mqtt_published(void) {
  EventBits_t bits = xEventGroupGetBits(sw_status_events);
  return ((bits & STATUS_MQTT_PUBLISHED) == STATUS_MQTT_PUBLISHED);
}

void sys_stat_set_mqtt_subscribed(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(sw_status_events, STATUS_MQTT_SUBSCRIBED);
  } else {
    xEventGroupClearBits(sw_status_events, STATUS_MQTT_SUBSCRIBED);
  }
}

int sys_stat_get_mqtt_subscribed(void) {
  EventBits_t bits = xEventGroupGetBits(sw_status_events);
  return ((bits & STATUS_MQTT_SUBSCRIBED) == STATUS_MQTT_SUBSCRIBED);
}

void sys_stat_set_mqtt_init_finished(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(sw_status_events, STATUS_MQTT_INIT_FINISHED);
  } else {
    xEventGroupClearBits(sw_status_events, STATUS_MQTT_INIT_FINISHED);
  }
}

int sys_stat_get_mqtt_init_finished(void) {
  EventBits_t bits = xEventGroupGetBits(sw_status_events);
  return ((bits & STATUS_MQTT_INIT_FINISHED) == STATUS_MQTT_INIT_FINISHED);
}

bool sys_stat_wait_for_swevent(int event, int timeout_ms) {
  TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS;
  EventBits_t bits = xEventGroupWaitBits(sw_status_events, event, pdFALSE, pdTRUE, xTicksToWait);
  return ((bits & event) == event);
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

int sys_stat_get_battery_model(void) {
  EventBits_t bits = xEventGroupGetBits(hw_status_events);
  return ((bits & STATUS_BATTERY) == STATUS_BATTERY);
}

void sys_stat_set_battery_model(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(hw_status_events, STATUS_BATTERY);
  } else {
    xEventGroupClearBits(hw_status_events, STATUS_BATTERY);
  }
}

void sys_stat_set_wifi_scanning(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(hw_status_events, STATUS_WIFI_SCANNING);
  } else {
    xEventGroupClearBits(hw_status_events, STATUS_WIFI_SCANNING);
  }
}

int sys_stat_get_wifi_scanning(void) {
  EventBits_t bits = xEventGroupGetBits(hw_status_events);
  return ((bits & STATUS_WIFI_SCANNING) == STATUS_WIFI_SCANNING);
}

void sys_stat_set_wifi_scan_done(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(hw_status_events, STATUS_WIFI_SCAN_DONE);
  } else {
    xEventGroupClearBits(hw_status_events, STATUS_WIFI_SCAN_DONE);
  }
}

int sys_stat_get_wifi_scan_done(void) {
  EventBits_t bits = xEventGroupGetBits(hw_status_events);
  return ((bits & STATUS_WIFI_SCAN_DONE) == STATUS_WIFI_SCAN_DONE);
}

bool sys_stat_wait_for_hwevent(int event, int timeout_ms) {
  TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS;
  EventBits_t bits = xEventGroupWaitBits(hw_status_events, event, pdFALSE, pdTRUE, xTicksToWait);
  return ((bits & event) == event);
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

int sys_stat_get_actuator_open(void) {
  EventBits_t bits = xEventGroupGetBits(actuator_status_events);
  return ((bits & STATUS_OPEN) == STATUS_OPEN);
}

void sys_stat_set_actuator_open(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(actuator_status_events, STATUS_OPEN);
  } else {
    xEventGroupClearBits(actuator_status_events, STATUS_OPEN);
  }
}

int sys_stat_get_actuator_err(void) {
  EventBits_t bits = xEventGroupGetBits(actuator_status_events);
  return ((bits & STATUS_ERROR) == STATUS_ERROR);
}

void sys_stat_set_actuator_err(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(actuator_status_events, STATUS_ERROR);
  } else {
    xEventGroupClearBits(actuator_status_events, STATUS_ERROR);
  }
}

// ----------------------------------------------------------------
// Irrigation status event group
// ----------------------------------------------------------------

int sys_stat_get_time_sync(void) {
  EventBits_t bits = xEventGroupGetBits(irrigation_status_events);
  return ((bits & STATUS_TIME_SYNC) == STATUS_TIME_SYNC);
}

void sys_stat_set_time_sync(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(irrigation_status_events, STATUS_TIME_SYNC);
  } else {
    xEventGroupClearBits(irrigation_status_events, STATUS_TIME_SYNC);
  }
}

int sys_stat_get_battery_level(void) {
  EventBits_t bits = xEventGroupGetBits(irrigation_status_events);
  return ((bits & STATUS_BATTERY_LEVEL) == STATUS_BATTERY_LEVEL);
}

void sys_stat_set_battery_level(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(irrigation_status_events, STATUS_BATTERY_LEVEL);
  } else {
    xEventGroupClearBits(irrigation_status_events, STATUS_BATTERY_LEVEL);
  }
}

int sys_stat_get_start_irrigation(void) {
  EventBits_t bits = xEventGroupGetBits(irrigation_status_events);
  return ((bits & STATUS_START_IRRIGATION) == STATUS_START_IRRIGATION);
}

void sys_stat_set_start_irrigation(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(irrigation_status_events, STATUS_START_IRRIGATION);
  } else {
    xEventGroupClearBits(irrigation_status_events, STATUS_START_IRRIGATION);
  }
}

int sys_stat_get_stop_irrigation(void) {
  EventBits_t bits = xEventGroupGetBits(irrigation_status_events);
  return ((bits & STATUS_STOP_IRRIGATION) == STATUS_STOP_IRRIGATION);
}

void sys_stat_set_stop_irrigation(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(irrigation_status_events, STATUS_STOP_IRRIGATION);
  } else {
    xEventGroupClearBits(irrigation_status_events, STATUS_STOP_IRRIGATION);
  }
}

/*----------------------------------------------------------------*/
/*                    Data logger Status                          */
/*----------------------------------------------------------------*/

int sys_stat_get_sdcard_fail(void) {
  EventBits_t bits = xEventGroupGetBits(data_logger_status_events);
  return ((bits & STATUS_SDCARD_FAIL) == STATUS_SDCARD_FAIL);
}

void sys_stat_set_sdcard_fail(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(data_logger_status_events, STATUS_SDCARD_FAIL);
  } else {
    xEventGroupClearBits(data_logger_status_events, STATUS_SDCARD_FAIL);
  }
}

int sys_stat_get_rs485_conn_fail(void) {
  EventBits_t bits = xEventGroupGetBits(data_logger_status_events);
  return ((bits & STATUS_RS485_CONN_FAIL) == STATUS_RS485_CONN_FAIL);
}

void sys_stat_set_rs485_conn_fail(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(data_logger_status_events, STATUS_RS485_CONN_FAIL);
  } else {
    xEventGroupClearBits(data_logger_status_events, STATUS_RS485_CONN_FAIL);
  }
}

int sys_stat_get_usb_copying(usb_stat_t usb_status) {
  switch (usb_status) {
    case USB_COPYING: {
      EventBits_t bits = xEventGroupGetBits(data_logger_status_events);
      return ((bits & STATUS_USB_COPYING) == STATUS_USB_COPYING);
    } break;
    case USB_COPY_FAIL: {
      EventBits_t bits = xEventGroupGetBits(data_logger_status_events);
      return ((bits & STATUS_USB_COPY_FAIL) == STATUS_USB_COPY_FAIL);
    } break;
    case USB_COPY_SUCCESS: {
      EventBits_t bits = xEventGroupGetBits(data_logger_status_events);
      return ((bits & STATUS_USB_COPY_SUCCESS) == STATUS_USB_COPY_SUCCESS);
    } break;
    default: return 0;
  }
}

void sys_stat_set_usb_copying(usb_stat_t usb_status, uint8_t status) {
  switch (usb_status) {
    case USB_COPYING: {
      if (!!status) {
        xEventGroupSetBits(data_logger_status_events, STATUS_USB_COPYING);
      } else {
        xEventGroupClearBits(data_logger_status_events, STATUS_USB_COPYING);
      }
    } break;
    case USB_COPY_FAIL: {
      if (!!status) {
        xEventGroupSetBits(data_logger_status_events, STATUS_USB_COPY_FAIL);
      } else {
        xEventGroupClearBits(data_logger_status_events, STATUS_USB_COPY_FAIL);
      }
    } break;
    case USB_COPY_SUCCESS: {
      if (!!status) {
        xEventGroupSetBits(data_logger_status_events, STATUS_USB_COPY_SUCCESS);
      } else {
        xEventGroupClearBits(data_logger_status_events, STATUS_USB_COPY_SUCCESS);
      }
    } break;
  }
}

int sys_stat_get_usb_disconnect_notify(void) {
  EventBits_t bits = xEventGroupGetBits(data_logger_status_events);
  return ((bits & STATUS_USB_DISCONN) == STATUS_USB_DISCONN);
}

void sys_stat_set_usb_disconnect_notify(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(data_logger_status_events, STATUS_USB_DISCONN);
  } else {
    xEventGroupClearBits(data_logger_status_events, STATUS_USB_DISCONN);
  }
}

int sys_stat_get_file_write_flag(void) {
  EventBits_t bits = xEventGroupGetBits(data_logger_status_events);
  return ((bits & STATUS_LOG_FILE_WRITE) == STATUS_LOG_FILE_WRITE);
}

void sys_stat_set_file_write_flag(uint8_t status) {
  if (!!status) {
    xEventGroupSetBits(data_logger_status_events, STATUS_LOG_FILE_WRITE);
  } else {
    xEventGroupClearBits(data_logger_status_events, STATUS_LOG_FILE_WRITE);
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

  actuator_status_events = xEventGroupCreate();
  if (actuator_status_events == NULL) {
    dbge("Failed to create event group for actuator status");
    return -1;
  }

  irrigation_status_events = xEventGroupCreate();
  if (irrigation_status_events == NULL) {
    dbge("Failed to create event group for irrigation status");
    return -1;
  }

  data_logger_status_events = xEventGroupCreate();
  if (data_logger_status_events == NULL) {
    dbge("Failed to create event group for data logger status");
    return -1;
  }

  return 0;
}
