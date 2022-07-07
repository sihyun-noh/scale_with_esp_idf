#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include "sysevent.h"
#include "syslog.h"
#include "monitoring.h"
#include "icmp_echo_api.h"
#include "event_ids.h"

static const char *TAG = "MONITOR";
static TaskHandle_t wifi_event_monitor_task_handle = NULL;
typedef struct task_item {
  uint8_t task_id;
  char task_name[16];
  uint8_t status;
  uint32_t heart_bit_count;
  TAILQ_ENTRY(task_item) next;
}task_item_t;

TAILQ_HEAD(task_item_head, task_item);
typedef struct task_queue {
  uint8_t task_num;
  struct task_item_head head;
}task_queue_t;

SemaphoreHandle_t monitor_semaphore;
static task_queue_t task_list;

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
    LOGE(TAG, "register ID : %d", entry->task_id);
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
      if (!add_to_task_list(&task_list, task_list.task_num, task_name)) {
        task_list.task_num++;
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
  while (entry_num >= 0) {
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
    SLOGE(TAG, "Heap critical level reached: %d", critical_level);
    SLOGE(TAG, "-------------------------------------------------------------");
    SLOGE(TAG, "heap_track - total_free_bytes : %d", heap_info.total_free_bytes);
    SLOGE(TAG, "heap_track - total_allocated_bytes : %d", heap_info.total_allocated_bytes);
    SLOGE(TAG, "heap_track - largest_free_block : %d", heap_info.largest_free_block);
    SLOGE(TAG, "heap_track - minimum_free_bytes : %d", heap_info.minimum_free_bytes);
    SLOGE(TAG, "heap_track - allocated_blocks : %d", heap_info.allocated_blocks);
    SLOGE(TAG, "heap_track - free_blocks : %d", heap_info.free_blocks);
    SLOGE(TAG, "heap_track - total_blocks : %d", heap_info.total_blocks);
    SLOGE(TAG, "-------------------------------------------------------------");
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

static void monitoring_task(void *pvParameters) {
  is_run_task_monitor_alarm(wifi_event_monitor_task_handle);
  uint8_t list_count = 0;
  int ret = -1;
  char res_event_msg[50] = { 0 };
  while (1) {
    is_run_task_heart_bit(wifi_event_monitor_task_handle, true);

    wifi_monitoring();

    heap_monitoring(HEAP_MONITOR_WARNING, HEAP_MONITOR_CRITICAL);

    task_monitoring(list_count);

    if (list_count == 0) {
      if (xSemaphoreTake(monitor_semaphore, portMAX_DELAY) == pdTRUE) {
        list_count = task_list.task_num;
        xSemaphoreGive(monitor_semaphore);
      }
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
