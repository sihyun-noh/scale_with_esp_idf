#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "msc.h"
#include "syslog.h"
#include "main.h"
#include "sys_status.h"
#include "file_copy.h"
#include "syscfg.h"
#include "sys_config.h"

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

static const char *TAG = "usb_msc_task";
static TaskHandle_t usb_msc_handle = NULL;

#define USBROOT "/usb"
#define CREATE_FILE_NAME_1 "/sen1"
#define CREATE_FILE_NAME_2 "/sen2"
#define CREATE_FILE_NAME_3 "/sen3"
#define SENSOR_COUNT 3
enum { SENSOR_1 = 0, SENSOR_2, SENSOR_3 };

typedef struct {
  DIR *dir;
  struct dirent *ent;
  struct stat sb;
  uint64_t to_file_size;
  uint64_t from_file_size;
  uint8_t file_num;
} file_ctx_t;

typedef struct node {
  char *dir_path;
  char *create_file_name;
  char *file_name;
} copy_ctx_t;

int file_operations(void) {
  char empty[10] = { 0 };
  char directory[30] = { 0 };
  char to_file_path[150] = { 0 };
  char from_file_path[150] = { 0 };
  char mac_address[16] = { 0 };

  file_ctx_t *file_data = (file_ctx_t *)malloc(sizeof(file_ctx_t));
  memset(file_data, 0, sizeof(file_ctx_t));

  copy_ctx_t *copy_data = (copy_ctx_t *)malloc(sizeof(copy_ctx_t));
  memset(copy_data, 0, sizeof(copy_ctx_t));

  syscfg_get(SYSCFG_I_MACADDRESS, SYSCFG_N_MACADDRESS, mac_address, sizeof(mac_address));
  strncpy(empty, mac_address + 4, sizeof(empty) - 2);
  snprintf(directory, sizeof(directory), "%s/%s", USBROOT, empty);
  LOGE(TAG, "dir__: %s", directory);

  bool directory_exists = stat((const char *)directory, &file_data->sb) == 0;
  if (!directory_exists) {
    if (mkdir((const char *)directory, 0777) != 0) {
      LOGE(TAG, "mkdir failed with errno: %s\n", strerror(errno));
    }
  }

  for (int i = 0; i < SENSOR_COUNT; i++) {
    switch (i) {
      case SENSOR_1: {
        copy_data->dir_path = SEN_DATA_PATH_1;
        copy_data->create_file_name = CREATE_FILE_NAME_1;
      } break;
      case SENSOR_2: {
        copy_data->dir_path = SEN_DATA_PATH_2;
        copy_data->create_file_name = CREATE_FILE_NAME_2;
      } break;
      case SENSOR_3: {
        copy_data->dir_path = SEN_DATA_PATH_3;
        copy_data->create_file_name = CREATE_FILE_NAME_3;
      } break;
      default: break;
    }

    file_data->dir = opendir(copy_data->dir_path);
    file_data->file_num = 0;
    while ((file_data->ent = readdir(file_data->dir)) != NULL) {
      file_data->file_num++;
      copy_data->file_name = (char *)malloc(sizeof(char) * 30);
      memset(copy_data->file_name, 0, sizeof(char) * 30);
      memcpy(copy_data->file_name, file_data->ent->d_name, strlen(file_data->ent->d_name));

      snprintf(to_file_path, sizeof(to_file_path), "%s%s_%d.csv", directory, copy_data->create_file_name,
               file_data->file_num);
      snprintf(from_file_path, sizeof(from_file_path), "%s/%s", copy_data->dir_path, copy_data->file_name);
      LOGE(TAG, "print file to path : %s\n", to_file_path);
      LOGE(TAG, "print file from path : %s\n", from_file_path);
      int res = file_copy(to_file_path, from_file_path);
      if (res != 0) {
        printf("  Error copying file (%d)\n", res);
        free(copy_data);
        free(file_data);
        return -1;
      }

      stat(from_file_path, &file_data->sb);
      file_data->from_file_size = file_data->sb.st_size;

      stat(to_file_path, &file_data->sb);
      file_data->to_file_size = file_data->sb.st_size;

      // checking the copied file to usb.
      if (file_data->to_file_size == file_data->from_file_size) {
        // delete file in spiffs
        LOGI(TAG, "file copy success! (file_num : %d)", file_data->file_num);
        // set_usb_copying(USB_COPYING, 0);
        set_usb_copying(USB_COPY_SUCCESS, 1);
        // OK;
      } else {
        LOGI(TAG, "to file : %lld, from file : %lld", file_data->to_file_size, file_data->from_file_size);
        LOGI(TAG, "file copy fail!");  // FAIL;
        // set_usb_copying(USB_COPYING, 0);
        set_usb_copying(USB_COPY_FAIL, 1);
      }
      free(copy_data->file_name);
    }
  }

  free(copy_data);
  free(file_data);
  return 0;
}

void usb_host_msc_task(void) {
  while(1) {
    uint8_t device_address = wait_for_msc_device();

    if (device_address != 0) {
      set_usb_disconnect_notify(0);
      set_usb_copying(USB_COPYING, 0);
      set_usb_copying(USB_COPY_FAIL, 0);
      set_usb_copying(USB_COPY_SUCCESS, 0);

      usb_device_init(device_address);

      while (is_log_file_write_flag()) {
        LOGE(TAG, "wait....");
        vTaskDelay(500 / portTICK_PERIOD_MS);
      }
      set_usb_copying(USB_COPYING, 1);
      file_operations();
      set_usb_disconnect_notify(1);

      while (!wait_for_event(DEVICE_DISCONNECTED, 200)) {}

      set_usb_disconnect_notify(0);
      set_usb_copying(USB_COPYING, 0);
      set_usb_copying(USB_COPY_FAIL, 0);
      set_usb_copying(USB_COPY_SUCCESS, 0);

      usb_device_uninit();
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  usb_msc_host_uninit();
}

void create_usb_host_msc_task(void) {
  uint16_t stack_size = 8192;
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  usb_msc_host_init();

  if (usb_msc_handle) {
    LOGI(TAG, "usb host msc task is alreay created");
    return;
  }

  xTaskCreate((TaskFunction_t)usb_host_msc_task, "usb_host_msc", stack_size, NULL, task_priority, &usb_msc_handle);
}
