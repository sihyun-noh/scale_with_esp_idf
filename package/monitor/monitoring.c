#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include "sysevent.h"
#include "syslog.h"

#include "monitoring.h"

static const char *TAG = "MONITOR";
static TaskHandle_t wifi_event_monitor_task_handle = NULL;

static void heap_tracing() {
  multi_heap_info_t heap_info;
  heap_caps_get_info(&heap_info, MALLOC_CAP_8BIT);
  LOGI(TAG, "-------------------------------------------------------------");
  LOGI(TAG, "heap_track - total_free_bytes : %d", heap_info.total_free_bytes);
  LOGI(TAG, "heap_track - total_allocated_bytes : %d", heap_info.total_allocated_bytes);
  LOGI(TAG, "heap_track - largest_free_block : %d", heap_info.largest_free_block);
  LOGI(TAG, "heap_track - minimum_free_bytes : %d", heap_info.minimum_free_bytes);
  LOGI(TAG, "heap_track - allocated_blocks : %d", heap_info.allocated_blocks);
  LOGI(TAG, "heap_track - free_blocks : %d", heap_info.free_blocks);
  LOGI(TAG, "heap_track - total_blocks : %d", heap_info.total_blocks);
  LOGI(TAG, "-------------------------------------------------------------");
}

static void wifi_status_tracing() {
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
  }

  ret = sysevent_get(IP_EVENT, IP_EVENT_STA_GOT_IP, &res_event_msg, sizeof(res_event_msg));
  if (!ret) {
    LOGI(TAG, "IP_EVENT_STA_GOT_IP : %s\n", res_event_msg);
    memset(&res_event_msg, 0, sizeof(res_event_msg));
  }
}

static void monitoring_task(void *pvParameters) {
  while (1) {
    wifi_status_tracing();
    heap_tracing();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
  wifi_event_monitor_task_handle = NULL;
}

bool carete_monitoring_task(void) {
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (!wifi_event_monitor_task_handle) {
    xTaskCreate(monitoring_task, "monitoring_task", 4096, NULL, task_priority, &wifi_event_monitor_task_handle);
    if (!wifi_event_monitor_task_handle) {
      LOGI(TAG, "Monitoring_task Task Start Failed!");
      return false;
    }
  }
  return true;
}
