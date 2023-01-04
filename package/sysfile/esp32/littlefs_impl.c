#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "esp_err.h"
#include "esp_littlefs.h"

#include "syslog.h"
#include "littlefs_impl.h"

const static char *TAG = "LittleFS";

int init_littlefs_impl(void) {
  LOGI(TAG, "Initializing LittleFS");

  esp_vfs_littlefs_conf_t conf = {
    .base_path = BASE_PATH, .partition_label = PARTITION_NAME, .format_if_mount_failed = true, .dont_mount = false
  };

  esp_err_t ret = esp_vfs_littlefs_register(&conf);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      LOGE(TAG, "Failed to mount or format filesystem");
    } else if (ret == ESP_ERR_NOT_FOUND) {
      LOGE(TAG, "Failed to find LittleFS parition");
    } else {
      LOGE(TAG, "Failed to initialize LittleFS");
    }
    return -1;
  }

  size_t total = 0, used = 0;
  ret = esp_littlefs_info(conf.partition_label, &total, &used);
  if (ret != ESP_OK) {
    LOGE(TAG, "Failed to get LittleFS partition information");
  } else {
    LOGI(TAG, "Partition size : total = %d, used = %d", total, used);
  }
  return 0;
}

int format_littlefs_impl(void) {
  esp_err_t ret = esp_littlefs_format(PARTITION_NAME);
  if (ret == ESP_FAIL) {
    LOGE(TAG, "Failed to format littlefs system");
    return -1;
  }
  return 0;
}

int show_file_impl() {
  int num = 0;
  DIR *d;
  struct dirent *dir;

  LOGI(TAG, "base dir = %s", BASE_PATH);

  d = opendir(BASE_PATH);
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

int write_log_data_to_file_impl(const char *log_file_name, const char *log_data) {
  FILE *fd = NULL;

  fd = fopen(log_file_name, "a");
  if (fd == NULL) {
    LOGE(TAG, "Failed to open file for appending");
    return -1;
  }
  int written = fwrite(log_data, strlen(log_data), 1, fd);
  fclose(fd);
  return written;
}
