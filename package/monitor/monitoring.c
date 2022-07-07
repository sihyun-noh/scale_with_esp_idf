#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include "sysevent.h"
#include "syslog.h"
#include "monitoring.h"
#include "icmp_echo_api.h"
#include "event_ids.h"

static const char *TAG = "MONITOR";
static TaskHandle_t wifi_event_monitor_task_handle = NULL;
typedef struct data_entry {
  uint8_t id;
  char pcName[16];
  uint8_t status;
  uint32_t heart_bit_count;
  TAILQ_ENTRY(data_entry) next;
}data_entry_t;

TAILQ_HEAD(data_entry_head, data_entry);
typedef struct data_queue {
  uint8_t entry_num;
  struct data_entry_head head;
}data_queue_t;

SemaphoreHandle_t monitor_semaphore;
static data_queue_t data_q;

static void init_inked_list(void) {
  TAILQ_INIT(&(data_q.head));
  data_q.entry_num = 0;
}

static int push_data_entry_last(data_queue_t *q, uint8_t id, char *data) {
  data_entry_t *entry = (data_entry_t *)calloc(1, sizeof(data_entry_t));
  if (entry) {
    entry->id = id;
    memcpy(entry->pcName, data, strlen(data));
    TAILQ_INSERT_TAIL(&(q->head), entry, next);
    LOGE(TAG, "register ID : %d", entry->id);
    return 0;
  }
  return -1;
}

data_entry_t *get_some_data(data_queue_t *q, uint8_t id) {
  data_entry_t *entry;
  TAILQ_FOREACH(entry, &(q->head), next) {
    if (entry->id == id) {
      return entry;
    }
  }
  return NULL;
}

data_entry_t *get_some_data_task_name(data_queue_t *q, char *name) {
  data_entry_t *entry;
  TAILQ_FOREACH(entry, &(q->head), next) {
    if (strcmp(entry->pcName, name) == 0) {
      return entry;
    }
  }
  return NULL;
}

static void remove_data(data_queue_t *q, data_entry_t *entry) {
  TAILQ_REMOVE((&q->head), entry, next);
  free(entry);
  q->entry_num--;
}

int is_run_task_monitor_remove(TaskHandle_t task_handle) {
  if (xSemaphoreTake(monitor_semaphore, portMAX_DELAY) == pdTRUE) {
    if (task_handle == NULL) {
      return -1;
    }
    char *task_name = pcTaskGetName(task_handle);
    data_entry_t *entry = get_some_data_task_name(&data_q, task_handle);

    if (eTaskGetState(task_handle) == eRunning) {
      LOGI(TAG, "curr_tasK_alarm : %s", task_name);
      remove_data(&data_q, entry);
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
      if (!push_data_entry_last(&data_q, data_q.entry_num, task_name)) {
        data_q.entry_num++;
      }
    }
    xSemaphoreGive(monitor_semaphore);
  }
  return 0;
}

void is_run_task_heart_bit(TaskHandle_t task_handle, uint8_t status) {
  data_entry_t *entry;
  data_queue_t *q = &data_q;
  char *task_name = pcTaskGetName(task_handle);
  TAILQ_FOREACH(entry, &(q->head), next) {
    if (strcmp(entry->pcName, task_name) == 0) {
      entry->status = status;
    }
  }
}

static void Task_Monitoring(int8_t entry_num) {
  data_entry_t *item = NULL;
  while (entry_num >= 0) {
    item = get_some_data(&data_q, entry_num);
    if (item == NULL) {
      LOGI(TAG, "invalid ID : %d", entry_num);
      entry_num--;

    } else {
      if (item->status) {
        item->status = false;
        item->heart_bit_count = 0;
      } else {
        item->heart_bit_count++;
      }

      if (item->heart_bit_count > TASK_MAX_COUNT) {
        LOGI(TAG, "task_ID : %d", item->id);
        LOGI(TAG, "curr_task_name : %s", &(item->pcName));
        LOGI(TAG, "heart_bit_count : %d", item->heart_bit_count);

        sysevent_set(TASK_MONITOR_EVENT, &(item->pcName));
      }

      entry_num--;
    }
  }
}

static void Heap_Monitoring(int warning_level, int critical_level) {
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

static void WIFI_Monitoring(void) {
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

static void Monitoring_task(void *pvParameters) {
  is_run_task_monitor_alarm(wifi_event_monitor_task_handle);
  uint8_t list_count = 0;
  int ret = -1;
  char res_event_msg[50] = { 0 };
  while (1) {
    is_run_task_heart_bit(wifi_event_monitor_task_handle, true);

    WIFI_Monitoring();

    Heap_Monitoring(HEAP_MONITOR_WARNING, HEAP_MONITOR_CRITICAL);

    Task_Monitoring(list_count);

    if (list_count == 0) {
      if (xSemaphoreTake(monitor_semaphore, portMAX_DELAY) == pdTRUE) {
        list_count = data_q.entry_num;
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
    xTaskCreate(Monitoring_task, "Monitoring_task", 4096, NULL, task_priority, &wifi_event_monitor_task_handle);
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