#include "syscfg.h"
#include "syslog.h"
#include "sysevent.h"
#include "sys_status.h"
#include "wifi_manager.h"
#include "monitoring.h"
#include "icmp_echo_api.h"
#include "event_ids.h"
#include "easy_setup.h"

#include "esp_wifi_types.h"

#include <freertos/FreeRTOS.h>
#include <freertos/projdefs.h>
#include <freertos/event_groups.h>

#define CHECK_SUCCESS 0
#define CHECK_FAILURE -1

#define READY 1
#define NOT_READY 0

#define PING_RETRY_COUNT 5          /* Ping retry count */
#define PING_ADDR_NUMBER 2          /* the number of ping address */
#define PING_FAIL_COUNT_EACH_ADDR 5 /* the fail times for each ping address */

#define MAX_INTERNET_CHECK_TIME 10800  // 3 hour // 900, 15 min
#define MIN_INTERNET_CHECK_TIME 60     // 1 min

#define ROUTER_CHECK_TIME 3600  // 1 hour // 600, 10min

#define DELAY_5SEC 5000     // 5sec
#define DELAY_1MIN 60000    // 1min
#define DELAY_10MIN 600000  // 10min

extern char *uptime(void);

static const char *TAG = "MONITOR";
static TaskHandle_t monitor_task_handle = NULL;

static TickType_t g_last_router_check_time = 0;
static TickType_t g_last_internet_check_time = 0;

static int g_internet_check_time = MAX_INTERNET_CHECK_TIME;

static int g_farmnet_ready = 0;
static int g_internet_ready = 0;

static int g_network_state;
static int g_rssi;
static int g_ping_cnt;

char *g_ping_addr[PING_ADDR_NUMBER] = { "8.8.8.8", "www.google.com" };

typedef enum {
  ON_NETWORK = 0,
  ON_INTERNET,
  NO_ROUTER_CONNECTION,
  NO_INTERNET_CONNECTION,
} network_status_t;

// Monitoring internal functions for network connection status

static void set_wifi_state(int network_state) {
  g_network_state = network_state;
}

static void set_wifi_led(int led_mode) {
  // TODO : Call HAL API for controlling LED status
  switch (led_mode) {
    case ON_NETWORK:
    case ON_INTERNET:
      if (is_wifi_fail()) {
        set_wifi_fail(0);
      }
      break;
    case NO_ROUTER_CONNECTION:
    case NO_INTERNET_CONNECTION:
      if (!is_wifi_fail()) {
        set_wifi_fail(1);
      }
      break;
  }
}

static int get_farmnet_state(void) {
  return g_farmnet_ready;
}

static int get_internet_state(void) {
  return g_internet_ready;
}

static void set_farmnet_state(int ready) {
  g_farmnet_ready = ready;
}

static void set_internet_state(int ready) {
  g_internet_ready = ready;
}

static bool check_ip_ready(void) {
  int ret = 0;
  char ip_addr[20] = { 0 };

  ret = get_sta_ipaddr(ip_addr, sizeof(ip_addr));
  if (ret == 0 && ip_addr[0]) {
    SLOGI(TAG, "station ip addr = %s", ip_addr);
    return true;
  }
  return false;
}

/*
 * Is it need to check router connection status
 * Returns true, if it need to check router connection, there are two cases,
 *               (1) first time, (2) after some mins (ROUTER_CHECK_TIME * 1000)
 * Returns false, otherwise
 *
 */
static bool need_to_check_router(void) {
  if (!g_last_router_check_time ||
      (xTaskGetTickCount() >= g_last_router_check_time + pdMS_TO_TICKS(ROUTER_CHECK_TIME * 1000))) {
    return true;
  }
  return false;
}

static int get_gateway_address(char *ip_addr, size_t ip_addr_len) {
  char gateway_addr[20] = { 0 };
  syscfg_get(CFG_DATA, "gateway", gateway_addr, sizeof(gateway_addr));
  if (gateway_addr[0]) {
    snprintf(ip_addr, ip_addr_len, "%s", gateway_addr);
    return 0;
  }
  return -1;
}

/*
 * Function that checking if farm network is alive or not using wifi api or ping command.
 * (1) Check if the sensor node is connected to the router every 30 seconds.
 * (2) Return the cached g_farmnet_ready value not actual data until the checking time 30 seconds is reached.
 */
static int check_farmnet(void) {
  int farmnet_ready = NOT_READY;

  ap_info_t ap_info = { 0 };
  int rc = get_ap_info(&ap_info);
  SLOGI(TAG, "get_ap_info , rc = %d, ssid = %s", rc, ap_info.ssid);
  if (rc == 0 && check_ip_ready()) {
    g_rssi = ap_info.rssi;
    farmnet_ready = READY;
  } else {
    farmnet_ready = NOT_READY;
  }

  // One more check if the router is alive using ping command.
  char router_ip[20] = { 0 };
  if (get_gateway_address(router_ip, sizeof(router_ip)) == 0) {
    SLOGI(TAG, "gateway router ip addr = %s", router_ip);
    for (int i = 0; i < PING_RETRY_COUNT; i++) {
      if (do_ping(router_ip, 3) == PING_OK) {
        farmnet_ready = READY;
        break;
      } else {
        farmnet_ready = NOT_READY;
      }
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  } else {
    farmnet_ready = NOT_READY;
    SLOGI(TAG, "Not found gateway router ip address!!!");
  }

  set_farmnet_state(farmnet_ready);

  g_last_router_check_time = xTaskGetTickCount();

  return farmnet_ready;
}

/*
 * Is it need to check internet connection status ?
 * Returns true, if it need to check internet connection, there are two cases,
 *               (1) first time, (2) after some mins (MIN/MAX_INTERNET_CHECK_TIME * 1000)
 * Returns false, otherwise.
 *
 */
static bool need_to_check_internet(void) {
  if (!g_last_internet_check_time ||
      (xTaskGetTickCount() >= g_last_internet_check_time + pdMS_TO_TICKS(g_internet_check_time * 1000))) {
    return true;
  }
  return false;
}

/*
 * Function that checking if the internet connection is alive or not using ping command
 * (1) Check if the internet connection status is OK every 3 min(180 sec) in normal state mode.
 * (2) If the internet connetion is lost, adjust the interval time (30 sec) which checking the connection status to
 * detect the recovery status rapidlly.
 */
static int check_internet(void) {
  int internet_ready = NOT_READY;

  if (get_farmnet_state() == READY) {
    for (int i = 0; i < PING_ADDR_NUMBER; i++) {
      for (;;) {
        if (do_ping(g_ping_addr[i], 3) == PING_OK) {
          internet_ready = READY;
          g_ping_cnt = 0;
          g_internet_check_time = MAX_INTERNET_CHECK_TIME;
          SLOGI(TAG, "Internet Ping Success...");
          break;
        } else {
          g_ping_cnt++;
          if (g_ping_cnt >= (PING_ADDR_NUMBER * PING_FAIL_COUNT_EACH_ADDR)) {
            internet_ready = NOT_READY;
            g_ping_cnt = 0;
            g_internet_check_time = MIN_INTERNET_CHECK_TIME;
            SLOGI(TAG, "Internet Ping Failure...");
            break;
          }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
      }
      if (internet_ready == READY) {
        break;
      }
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    g_last_internet_check_time = xTaskGetTickCount();
  }

  set_internet_state(internet_ready);

  return internet_ready;
}

static void monitoring_task(void *pvParameters) {
  int delay_ms = DELAY_5SEC;

  while (1) {
    // Do not need to check if the device's wifi connection status during the easy setup progress.
    // Only check if connection status of farm network or internet after device is onboarding on the network.
    if (is_device_configured() && (wifi_get_current_mode() == WIFI_MODE_STA)) {
      if (need_to_check_router() == true) {
        // First, check to see if the farmnet wifi connection status.
        if (check_farmnet() == READY) {
          SLOGI(TAG, "Success ping to farmnet");
          set_wifi_state(ON_NETWORK);
          set_wifi_led(ON_NETWORK);
          delay_ms = DELAY_10MIN;
        } else {
          // Set internet connection flag as lost
          // Reset g_last_internet_check_time
          if (get_internet_state() != NOT_READY) {
            set_internet_state(NOT_READY);
          }
          g_last_internet_check_time = 0;

          // Check if previous internet connection status is available(OK), if OK, set internet check time as minimum
          // value.

          // Reset g_last_router_check_time
          g_last_router_check_time = 0;

          SLOGI(TAG, "Failure ping to farmnet");

          set_wifi_state(NO_ROUTER_CONNECTION);
          set_wifi_led(NO_ROUTER_CONNECTION);

          // TODO: Run easy_setup task to connect to the router
          // The most of this case is caused by router is dead.
          set_device_onboard(0);
        }
      }
      if (need_to_check_internet() == true) {
        // Second, check to see if internet connection status after router status is OK
        if (check_internet() == READY) {
          // Internet is available, check NTP for local time and other stuff
          // Check if NTP is activated, if not, do NTP process

          // Check the network connection and signal strength
          set_wifi_state(ON_INTERNET);
        } else {
          // Internet is not available, nothing to do
          // Set the flag as internet connection lost and blink LED as defined color.
          set_wifi_state(NO_INTERNET_CONNECTION);
          set_wifi_led(NO_INTERNET_CONNECTION);
        }
      }
    }

    if (is_device_configured() && !is_device_onboard()) {
      SLOGI(TAG, "Device is disconnected from the farm network!!!");
    }

    vTaskDelay(pdMS_TO_TICKS(delay_ms));
  }

  vTaskDelete(NULL);
  monitor_task_handle = NULL;
}

int create_monitoring_task(void) {
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (!monitor_task_handle) {
    xTaskCreatePinnedToCore(monitoring_task, "monitoring_task", 8192, NULL, task_priority, &monitor_task_handle, 0);
    if (!monitor_task_handle) {
      SLOGI(TAG, "Monitoring_task Task Start Failed!");
      return -1;
    }
  }
  return 0;
}

int monitoring_init(void) {
  return create_monitoring_task();
}

TaskHandle_t get_monitoring_task_handle(void) {
  return monitor_task_handle;
}
