/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_log.h"
#include "usb/usb_host.h"
#include "msc_host.h"
#include "msc_host_vfs.h"
#include "ffconf.h"
#include "esp_vfs.h"
#include "errno.h"
#include "hal/usb_hal.h"
#include "driver/gpio.h"
#include <esp_vfs_fat.h>
#include "msc.h"

#include "sys_status.h"

#define READY_TO_UNINSTALL (HOST_NO_CLIENT | HOST_ALL_FREE)

static const char *TAG = "usb_host_msc";

static EventGroupHandle_t usb_flags;
static msc_host_device_handle_t msc_device;
static msc_host_vfs_handle_t vfs_handle;
static msc_host_device_info_t info;
static BaseType_t task_created;

static void msc_event_cb(const msc_host_event_t *event, void *arg) {
  if (event->event == MSC_DEVICE_CONNECTED) {
    ESP_LOGI(TAG, "MSC device connected");
    // Obtained USB device address is placed after application events
    xEventGroupSetBits(usb_flags, DEVICE_CONNECTED | (event->device.address << 4));
  } else if (event->event == MSC_DEVICE_DISCONNECTED) {
    xEventGroupSetBits(usb_flags, DEVICE_DISCONNECTED);
    ESP_LOGI(TAG, "MSC device disconnected");
  }
}

static void print_device_info(msc_host_device_info_t *info) {
  const size_t megabyte = 1024 * 1024;
  uint64_t capacity = ((uint64_t)info->sector_size * info->sector_count) / megabyte;

  printf("Device info:\n");
  printf("\t Capacity: %llu MB\n", capacity);
  printf("\t Sector size: %" PRIu32 "\n", info->sector_size);
  printf("\t Sector count: %" PRIu32 "\n", info->sector_count);
  printf("\t PID: 0x%4X \n", info->idProduct);
  printf("\t VID: 0x%4X \n", info->idVendor);
  wprintf(L"\t iProduct: %S \n", info->iProduct);
  wprintf(L"\t iManufacturer: %S \n", info->iManufacturer);
  wprintf(L"\t iSerialNumber: %S \n", info->iSerialNumber);
}

// Handles common USB host library events
static void handle_usb_events(void *args) {
  while (1) {
    uint32_t event_flags;
    usb_host_lib_handle_events(portMAX_DELAY, &event_flags);

    // Release devices once all clients has deregistered
    if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
      usb_host_device_free_all();
      xEventGroupSetBits(usb_flags, HOST_NO_CLIENT);
    }
    // Give ready_to_uninstall_usb semaphore to indicate that USB Host library
    // can be deinitialized, and terminate this task.
    if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
      xEventGroupSetBits(usb_flags, HOST_ALL_FREE);
    }
  }

  vTaskDelete(NULL);
}

uint8_t wait_for_msc_device(void) {
  EventBits_t event;
  const TickType_t xTicksToWait = 200 / portTICK_PERIOD_MS;

  //  ESP_LOGI(TAG, "Waiting for USB stick to be connected");
  //  event = xEventGroupWaitBits(usb_flags, DEVICE_CONNECTED | DEVICE_ADDRESS_MASK, pdTRUE, pdFALSE, portMAX_DELAY);
  event = xEventGroupWaitBits(usb_flags, DEVICE_CONNECTED | DEVICE_ADDRESS_MASK, pdTRUE, pdFALSE, xTicksToWait);

  if ((event & DEVICE_CONNECTED) == DEVICE_CONNECTED) {
    ESP_LOGI(TAG, "connection...");
    // Extract USB device address from event group bits
    return (event & DEVICE_ADDRESS_MASK) >> 4;
  } else {
    return 0;
  }
}

bool wait_for_event(app_event_t event_stat, uint32_t time_ms) {
  EventBits_t event = event_stat;
  TickType_t timeout = time_ms;
  return xEventGroupWaitBits(usb_flags, event, pdTRUE, pdTRUE, timeout) & event;
}

int usb_msc_host_init(void) {
  usb_flags = xEventGroupCreate();
  assert(usb_flags);

  const usb_host_config_t host_config = { .intr_flags = ESP_INTR_FLAG_LEVEL1 };
  if ((usb_host_install(&host_config)) != ESP_OK) {
    ESP_ERROR_CHECK(usb_host_install(&host_config));
    return -1;
  }
  task_created = xTaskCreate(handle_usb_events, "usb_events", 4095, NULL, 2, NULL);

  assert(task_created);

  const msc_host_driver_config_t msc_config = {
    .create_backround_task = true,
    .task_priority = 5,
    .stack_size = 4096,
    .callback = msc_event_cb,
  };
  if ((msc_host_install(&msc_config)) != ESP_OK) {
    ESP_ERROR_CHECK(msc_host_install(&msc_config));
    return -1;
  }

  return 0;
}

int usb_msc_host_uninit(void) {
  ESP_LOGI(TAG, "Uninitializing USB ...");
  if ((msc_host_uninstall()) != ESP_OK) {
    ESP_ERROR_CHECK(msc_host_uninstall());
    return -1;
  }
  wait_for_event(READY_TO_UNINSTALL, portMAX_DELAY);
  if ((usb_host_uninstall()) != ESP_OK) {
    ESP_ERROR_CHECK(usb_host_uninstall());
    return -1;
  }
  ESP_LOGI(TAG, "Done");
  return 0;
}

int usb_device_init(uint8_t device_address) {
  const esp_vfs_fat_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 5,
    .allocation_unit_size = 1024,
  };
  if ((msc_host_install_device(device_address, &msc_device)) != ESP_OK) {
    ESP_ERROR_CHECK(msc_host_install_device(device_address, &msc_device));
    return -1;
  }
  msc_host_print_descriptors(msc_device);

  if ((msc_host_get_device_info(msc_device, &info)) != ESP_OK) {
    ESP_ERROR_CHECK(msc_host_get_device_info(msc_device, &info));
    return -1;
  }
  print_device_info(&info);
  if ((msc_host_vfs_register(msc_device, "/usb", &mount_config, &vfs_handle)) != ESP_OK) {
    ESP_ERROR_CHECK(msc_host_vfs_register(msc_device, "/usb", &mount_config, &vfs_handle));
    return -1;
  }
  return 0;
}

int usb_device_uninit(void) {
  xEventGroupClearBits(usb_flags, READY_TO_UNINSTALL);
  if ((msc_host_vfs_unregister(vfs_handle)) != ESP_OK) {
    ESP_ERROR_CHECK(msc_host_vfs_unregister(vfs_handle));
    return -1;
  }
  if ((msc_host_uninstall_device(msc_device)) != ESP_OK) {
    ESP_ERROR_CHECK(msc_host_uninstall_device(msc_device));
    return -1;
  }
  return 0;
}

