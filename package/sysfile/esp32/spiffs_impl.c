/**
 * @file spiffs_impl.c
 *
 * @brief using SPIFFS is provided in the "storage/spiffs" directory.
 * This is initializes and mounts a SPIFFS partition, then writes and reads data from it using POSIX and C library APIs.
 *
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_spiffs.h"
#include <dirent.h>

#include "syslog.h"
#include "spiffs_impl.h"

static const char *TAG = "spiffs";

static char s_partition_name[80];
static char s_root_path[256];

int init_spiffs_impl(const char *partition_name, const char *root_path) {
  LOGI(TAG, "Initializing SPIFFS");
  esp_vfs_spiffs_conf_t conf = {
    .base_path = root_path, .partition_label = NULL, .max_files = 5, .format_if_mount_failed = true
  };

  snprintf(s_partition_name, sizeof(s_partition_name), "%s", partition_name);
  snprintf(s_root_path, sizeof(s_root_path), "%s", root_path);

  esp_err_t ret = esp_vfs_spiffs_register(&conf);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      LOGE(TAG, "Failed to mount or format filesystem");
    } else if (ret == ESP_ERR_NOT_FOUND) {
      LOGE(TAG, "Failed to find SPIFFS partition");
    } else {
      LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
    }
    return -1;
  }

  size_t total = 0, used = 0;
  ret = esp_spiffs_info(conf.partition_label, &total, &used);
  if (ret != ESP_OK) {
    LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
  } else {
    LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
  }
  return 0;
}

int format_spiffs_impl() {
  esp_err_t ret = esp_spiffs_format(NULL);
  if (ret == ESP_FAIL) {
    LOGI(TAG, "Failed to format filesystem");
    return -1;
  }
  return 0;
}

int show_file_spiffs_impl(void) {
  int num = 0;
  DIR *d;
  struct dirent *dir;

  d = opendir(s_root_path);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      LOGI(TAG, "%d : %s\n", ++num, dir->d_name);
    }
    closedir(d);
    return 0;
  } else {
    LOGI(TAG, "Failed to show file_log name");
    return -1;
  }
}

int write_log_data_to_file_spiffs_impl(const char *log_file_name, const char *log_data) {
  FILE *fd = NULL;
  fd = fopen(log_file_name, "a");
  if (fd == NULL) {
    LOGE(TAG, "Failed to open file for writing");
    return -1;
  }
  fprintf(fd, log_data);
  return fclose(fd);
}
