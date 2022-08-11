#include "syscfg.h"
#include "syslog.h"
#include "sysevent.h"
#include "sys_status.h"
#include "wifi_manager.h"
#include "monitoring.h"
#include "icmp_echo_api.h"
#include "event_ids.h"
#include "easy_setup.h"
#include "filelog.h"

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#define CHECK_SUCCESS 0
#define CHECK_FAILURE -1

#define READY 1
#define NOT_READY 0

#define PING_ADDR_NUMBER 1          /* the number of ping address */
#define PING_FAIL_COUNT_EACH_ADDR 3 /* the fail times for each ping address */

#define MAX_INTERNET_CHECK_TIME 300  // 5 min
#define MIN_INTERNET_CHECK_TIME 60   // 1 min

#define ROUTER_CHECK_TIME 120  // 2 min

static const char *TAG = "MONITOR";
static TaskHandle_t wifi_event_monitor_task_handle = NULL;

static TickType_t g_last_router_check_time = 0;
static TickType_t g_last_internet_check_time = 0;

static int g_internet_check_time = MAX_INTERNET_CHECK_TIME;

static int g_farmnet_ready = 0;
static int g_internet_ready = 0;

static int g_network_state;
static int g_rssi;
static int g_ping_cnt;

char *g_ping_addr[PING_ADDR_NUMBER] = { "8.8.8.8" };

typedef enum {
  ON_NETWORK = 0,
  ON_INTERNET,
  NO_ROUTER_CONNECTION,
  NO_INTERNET_CONNECTION,
} network_status_t;

typedef struct task_item {
  uint8_t task_id;
  char task_name[16];
  uint8_t status;
  uint32_t heart_bit_count;
  TAILQ_ENTRY(task_item) next;
} task_item_t;

TAILQ_HEAD(task_item_head, task_item);
typedef struct task_queue {
  uint8_t task_num;
  struct task_item_head head;
} task_queue_t;

SemaphoreHandle_t monitor_semaphore;
static task_queue_t task_list;

static int get_task_count(void) {
  return task_list.task_num;
}

static void init_inked_list(void) {
  TAILQ_INIT(&(task_list.head));
  task_list.task_num = 0;
}

static int add_to_task_list(task_queue_t *q, uint8_t id, char *data) {
  task_item_t *entry = (task_item_t *)calloc(1, sizeof(task_item_t));
  if (entry) {
    entry->task_id = id;
    memcpy(entry->task_name, data, strlen(data));
    TAILQ_INSERT_TAIL(&(q->head), entry, next);
    LOGI(TAG, "register ID : %d", entry->task_id);
    return 0;
  }
  return -1;
}

task_item_t *get_task_item(task_queue_t *q, uint8_t id) {
  task_item_t *entry;
  TAILQ_FOREACH(entry, &(q->head), next) {
    if (entry->task_id == id) {
      return entry;
    }
  }
  return NULL;
}

task_item_t *get_task_item_with_name(task_queue_t *q, char *name) {
  task_item_t *entry;
  TAILQ_FOREACH(entry, &(q->head), next) {
    if (strcmp(entry->task_name, name) == 0) {
      return entry;
    }
  }
  return NULL;
}

static void remove_to_task_list(task_queue_t *q, task_item_t *entry) {
  TAILQ_REMOVE((&q->head), entry, next);
  free(entry);
  q->task_num--;
}

int is_run_task_monitor_remove(TaskHandle_t task_handle) {
  if (xSemaphoreTake(monitor_semaphore, portMAX_DELAY) == pdTRUE) {
    if (task_handle == NULL) {
      return -1;
    }
    char *task_name = pcTaskGetName(task_handle);
    task_item_t *entry = get_task_item_with_name(&task_list, task_handle);

    if (eTaskGetState(task_handle) == eRunning) {
      LOGI(TAG, "curr_tasK_alarm : %s", task_name);
      remove_to_task_list(&task_list, entry);
    }
    xSemaphoreGive(monitor_semaphore);
  }
  return 0;
}

int is_run_task_monitor_alarm(TaskHandle_t task_handle) {
  if (xSemaphoreTake(monitor_semaphore, portMAX_DELAY) == pdTRUE) {
    if (task_handle == NULL) {
      return -1;
    }
    char *task_name = pcTaskGetName(task_handle);
    if (eTaskGetState(task_handle) == eRunning) {
      LOGI(TAG, "curr_tasK_alarm : %s", task_name);
      task_list.task_num++;
      if (add_to_task_list(&task_list, task_list.task_num, task_name) == -1) {
        task_list.task_num--;
      }
    }
    xSemaphoreGive(monitor_semaphore);
  }
  return 0;
}

void is_run_task_heart_bit(TaskHandle_t task_handle, uint8_t status) {
  task_item_t *entry;
  task_queue_t *q = &task_list;
  char *task_name = pcTaskGetName(task_handle);
  TAILQ_FOREACH(entry, &(q->head), next) {
    if (strcmp(entry->task_name, task_name) == 0) {
      entry->status = status;
    }
  }
}

static void task_monitoring(int8_t entry_num) {
  task_item_t *entry = NULL;
  while (entry_num > 0) {
    entry = get_task_item(&task_list, entry_num);
    if (entry == NULL) {
      LOGI(TAG, "invalid ID : %d", entry_num);
      entry_num--;

    } else {
      if (entry->status) {
        entry->status = false;
        entry->heart_bit_count = 0;
      } else {
        entry->heart_bit_count++;
      }

      if (entry->heart_bit_count > TASK_MAX_COUNT) {
        LOGI(TAG, "task_ID : %d", entry->task_id);
        LOGI(TAG, "curr_task_name : %s", &(entry->task_name));
        LOGI(TAG, "heart_bit_count : %d", entry->heart_bit_count);

        sysevent_set(TASK_MONITOR_EVENT, &(entry->task_name));
      }

      entry_num--;
    }
  }
}

static void heap_monitoring(int warning_level, int critical_level) {
  char total_size[10] = { 0 };
  static int minHeap = 0;
  multi_heap_info_t heap_info;
  heap_caps_get_info(&heap_info, MALLOC_CAP_8BIT);
  uint32_t freeHeap = heap_info.total_free_bytes;

  if (minHeap == 0 || freeHeap < minHeap) {
    minHeap = freeHeap;
  }
  if (minHeap <= critical_level) {
    SLOGI(TAG, "Heap critical level reached: %d", critical_level);
    SLOGI(TAG, "-------------------------------------------------------------");
    SLOGI(TAG, "heap_track - total_free_bytes : %d", heap_info.total_free_bytes);
    SLOGI(TAG, "heap_track - total_allocated_bytes : %d", heap_info.total_allocated_bytes);
    SLOGI(TAG, "heap_track - largest_free_block : %d", heap_info.largest_free_block);
    SLOGI(TAG, "heap_track - minimum_free_bytes : %d", heap_info.minimum_free_bytes);
    SLOGI(TAG, "heap_track - allocated_blocks : %d", heap_info.allocated_blocks);
    SLOGI(TAG, "heap_track - free_blocks : %d", heap_info.free_blocks);
    SLOGI(TAG, "heap_track - total_blocks : %d", heap_info.total_blocks);
    SLOGI(TAG, "-------------------------------------------------------------");
    snprintf(total_size, sizeof(total_size), "%d", heap_info.total_free_bytes);
    sysevent_set(HEAP_CRITICAL_LEVEL_WARNING_EVENT, (char *)total_size);

  } else if (minHeap <= warning_level) {
    SLOGW(TAG, "Heap warning level reached: %d", warning_level);
    SLOGW(TAG, "-------------------------------------------------------------");
    SLOGW(TAG, "heap_track - total_free_bytes : %d", heap_info.total_free_bytes);
    SLOGW(TAG, "heap_track - total_allocated_bytes : %d", heap_info.total_allocated_bytes);
    SLOGW(TAG, "heap_track - largest_free_block : %d", heap_info.largest_free_block);
    SLOGW(TAG, "heap_track - minimum_free_bytes : %d", heap_info.minimum_free_bytes);
    SLOGW(TAG, "heap_track - allocated_blocks : %d", heap_info.allocated_blocks);
    SLOGW(TAG, "heap_track - free_blocks : %d", heap_info.free_blocks);
    SLOGW(TAG, "heap_track - total_blocks : %d", heap_info.total_blocks);
    SLOGW(TAG, "-------------------------------------------------------------");
    snprintf(total_size, sizeof(total_size), "%d", heap_info.total_free_bytes);
    sysevent_set(HEAP_WARNING_LEVEL_WARNING_EVENT, (char *)total_size);
  }
}

// Monitoring internal functions for network connection status

static void set_wifi_state(int network_state) {
  g_network_state = network_state;
}

static void set_wifi_led(int led_mode) {
  // TODO : Call HAL API for controlling LED status
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
    LOGI(TAG, "station ip addr = %s", ip_addr);
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

/*
 * Function that checking if farm network is alive or not using wifi api or ping command.
 * (1) Check if the sensor node is connected to the router every 30 seconds.
 * (2) Return the cached g_farmnet_ready value not actual data until the checking time 30 seconds is reached.
 */
static int check_farmnet(void) {
  int farmnet_ready = NOT_READY;

  ap_info_t ap_info = { 0 };
  int rc = get_ap_info(&ap_info);
  LOGI(TAG, "get_ap_info , rc = %d, ssid = %s", rc, ap_info.ssid);
  if (rc == 0 && check_ip_ready()) {
    g_rssi = ap_info.rssi;
    farmnet_ready = READY;
  } else {
    farmnet_ready = NOT_READY;
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
    for (;;) {
      if (do_ping(g_ping_addr[0], 3) == PING_OK) {
        internet_ready = READY;
        g_ping_cnt = 0;
        g_internet_check_time = MAX_INTERNET_CHECK_TIME;
        LOGI(TAG, "Internet Ping Success...");
        break;
      } else {
        g_ping_cnt++;
        if (g_ping_cnt >= (PING_ADDR_NUMBER * PING_FAIL_COUNT_EACH_ADDR)) {
          internet_ready = NOT_READY;
          g_ping_cnt = 0;
          g_internet_check_time = MIN_INTERNET_CHECK_TIME;
          LOGI(TAG, "Internet Ping Failure...");
          break;
        }
      }
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    g_last_internet_check_time = xTaskGetTickCount();
  }

  set_internet_state(internet_ready);

  return internet_ready;
}

static void wifi_monitoring(void) {
  char res_event_msg[50] = { 0 };
  int ret = -1;

  ret = sysevent_get(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &res_event_msg, sizeof(res_event_msg));
  if (!ret) {
    LOGI(TAG, "WIFI_EVENT_STA_CONNECTED : %s\n", res_event_msg);
    memset(&res_event_msg, 0, sizeof(res_event_msg));
  }

  ret = sysevent_get(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &res_event_msg, sizeof(res_event_msg));
  if (!ret) {
    LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED : %s\n", res_event_msg);
    memset(&res_event_msg, 0, sizeof(res_event_msg));

    SLOGI(TAG, "start_ping_google");
    if ((do_ping("www.google.com", 3) == PING_ERR_UNKNOWN_HOST)) {
      sysevent_set(NO_INTERNET_EVENT, (char *)res_event_msg);
    }
  }

  ret = sysevent_get(IP_EVENT, IP_EVENT_STA_GOT_IP, &res_event_msg, sizeof(res_event_msg));
  if (!ret) {
    LOGI(TAG, "IP_EVENT_STA_GOT_IP : %s\n", res_event_msg);
    memset(&res_event_msg, 0, sizeof(res_event_msg));
  }
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

static void monitoring_task(void *pvParameters) {
  int ret = -1;
  uint8_t task_count = 0;
  char res_event_msg[50] = { 0 };

  is_run_task_monitor_alarm(wifi_event_monitor_task_handle);
  task_count = get_task_count();

  while (1) {
    // Do not need to check if the device's wifi connection status during the easy setup progress.
    // Only check if connection status of farm network or internet after device is onboarding on the network.
    // wifi_monitoring();
    if (is_device_configured() && (wifi_get_current_mode() == WIFI_MODE_STA)) {
      if (need_to_check_router() == true) {
        // First, check to see if the farmnet wifi connection status.
        if (check_farmnet() == READY) {
          // One more check if the router is alive using ping command.
          char router_ip[20] = { 0 };
          if (get_gateway_address(router_ip, sizeof(router_ip)) == 0) {
            LOGI(TAG, "gateway router ip addr = %s", router_ip);
            if (do_ping(router_ip, 3) == PING_OK) {
              set_farmnet_state(READY);
              set_wifi_state(ON_NETWORK);
              SLOGI(TAG, "Success ping to farmnet");
              FLOGI(TAG, "Success ping to farmnet");
            } else {
              set_farmnet_state(NOT_READY);
            }
          } else {
            SLOGI(TAG, "Not found gateway router ip address!!!");
            FLOGI(TAG, "Not found gateway router ip address!!!");
          }
        }
        if (get_farmnet_state() == NOT_READY) {
          // Set internet connection flag as lost
          // Reset g_last_internet_check_time
          set_internet_state(NOT_READY);
          g_last_internet_check_time = 0;

          // Check if previous internet connection status is available(OK), if OK, set internet check time as minimum
          // value.

          // Reset g_last_router_check_time
          g_last_router_check_time = 0;

          SLOGI(TAG, "Failure ping to farmnet");
          FLOGI(TAG, "Failure ping to farmnet");

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
      LOGI(TAG, "Device is disconnected from the farm network!!!");
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

    is_run_task_heart_bit(wifi_event_monitor_task_handle, true);

    heap_monitoring(HEAP_MONITOR_WARNING, HEAP_MONITOR_CRITICAL);

    task_monitoring(task_count);

    if (xSemaphoreTake(monitor_semaphore, portMAX_DELAY) == pdTRUE) {
      task_count = task_list.task_num;
      xSemaphoreGive(monitor_semaphore);
    }

    ret = sysevent_get(SYSEVENT_BASE, TASK_MONITOR_EVENT, &res_event_msg, sizeof(res_event_msg));
    if (!ret) {
      LOGI(TAG, "TASK_MONITOR_EVENT : %s\n", res_event_msg);
      memset(&res_event_msg, 0, sizeof(res_event_msg));
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  vTaskDelete(NULL);
  wifi_event_monitor_task_handle = NULL;
}

int create_monitoring_task(void) {
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (!wifi_event_monitor_task_handle) {
    xTaskCreate(monitoring_task, "monitoring_task", 4096, NULL, task_priority, &wifi_event_monitor_task_handle);
    if (!wifi_event_monitor_task_handle) {
      LOGI(TAG, "Monitoring_task Task Start Failed!");
      return -1;
    }
  }
  return 0;
}

int monitoring_init(void) {
  monitor_semaphore = xSemaphoreCreateMutex();
  if (monitor_semaphore == NULL) {
    return -1;
  }
  init_inked_list();
  return create_monitoring_task();
}
