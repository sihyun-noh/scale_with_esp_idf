/**
 * @file filelog.c
 *
 * @brief It is a program that manages the creation and deletion of log files.
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/unistd.h>

#include "filelog.h"
#include "log.h"
#include "sysfile.h"
#include "sdkconfig.h"

#define FILE_LOG_MAX_BUFF_SIZE 1024
#define FILE_LOG_MAX_MSG_SIZE (FILE_LOG_MAX_BUFF_SIZE)
#define FILE_LOG_MAX_FILE_SIZE (FILE_LOG_MAX_MSG_SIZE * 500)
#define DIR_COUNT 3
#define FILE_COUNT 3

static const char *TAG = "FILE_LOG";

int g_file_log_num = FILE_COUNT;

typedef struct filelog {
  uint8_t file_num;
  size_t latest_file_size;
  char latest_file[30];
  char oldest_file[30];
  char filepath[50];
} file_log_t;

static file_log_t file_ctx;

/* Check the file name, number of files, latest files, and oldest files in the file directory. */

static int file_status_check(const char *dirpath) {
  uint64_t latest_cmp_date = 0;
  uint64_t oldest_cmp_date = 0;
  uint64_t temp_date = 0;
  char filepath[50] = { 0 };
  char temp_buf[50] = { 0 };
  char temp_date_buf[20] = { 0 };
  uint8_t file_count = 0;

  struct dirent *entry;
  struct stat file_stat;

  DIR *dir = opendir(dirpath);

  if (!dir) {
    LOGE(TAG, "Failed to stat dir : %s", dirpath);
    return -1;
  }

  while ((entry = readdir(dir)) != NULL) {
    strlcpy(temp_buf, entry->d_name, strlen(entry->d_name) + 1);

    memset(temp_date_buf, 0, sizeof(temp_date_buf));
    char *tok = strtok(temp_buf, "_");

    char *tok_2 = strtok(tok, "-");
    while (tok_2 != NULL) {
      strncat(temp_date_buf, tok_2, strlen(tok_2));
      tok_2 = strtok(NULL, "-");
    }

    temp_date = atoll(temp_date_buf);

    if (oldest_cmp_date == 0 && oldest_cmp_date == 0) {
      oldest_cmp_date = temp_date;
      latest_cmp_date = temp_date;
    }

    if (temp_date <= oldest_cmp_date) {
      oldest_cmp_date = temp_date;
      strlcpy(file_ctx.oldest_file, entry->d_name, strlen(entry->d_name) + 1);
    }

    if (temp_date >= latest_cmp_date) {
      latest_cmp_date = temp_date;
      strlcpy(file_ctx.latest_file, entry->d_name, strlen(entry->d_name) + 1);

      strlcpy(filepath, dirpath, strlen(dirpath) + 1);
      strncat(filepath, "/", 2);
      strncat(filepath, entry->d_name, strlen(entry->d_name));
      // LOGE(TAG, "filepath : %s", filepath);

      if (stat(filepath, &file_stat) == -1) {
        LOGE(TAG, "stat error. file path : %s", filepath);
      } else {
        file_ctx.latest_file_size = file_stat.st_size;
        strlcpy(file_ctx.filepath, filepath, strlen(filepath) + 1);
      }
    }
    file_count++;
  }
  file_ctx.file_num = file_count;
  // LOGI(TAG, "latest file : %s", file_ctx.latest_file);
  // LOGI(TAG, "oldest file : %s", file_ctx.oldest_file);
  // LOGI(TAG, "file_num : %d", file_ctx.file_num);
  // LOGI(TAG, "file_path : %s", file_ctx.filepath);

  closedir(dir);
  return 0;
}

/* Extracts the elements necessary to create a file name and
 * combines them to create a file name. */
static char *file_path_name(const char *path) {
  char temp_buf[50] = { 0 };
  char time_date_buf[30] = { 0 };
  char *time_date;
  char *newfilepath;

  newfilepath = malloc(sizeof(temp_buf));
  time_date = log_timestamp();
  strlcpy(time_date_buf, time_date, 17);  // 2022-00-00 10:20

  char *tok = strtok(time_date_buf, ":");  // 2022-00-00 10
  char *tok_1 = strtok(NULL, ":");         // 20
  char *tok_2 = strtok(tok, " ");          // 2022-00-00
  char *tok_3 = strtok(NULL, " ");         // 10

  memset(temp_buf, 0, sizeof(temp_buf));
  strlcpy(temp_buf, path, strlen(path) + 1);
  strncat(temp_buf, "/", 2);
  strncat(temp_buf, tok_2, strlen(tok_2));
  strncat(temp_buf, "-", 2);
  strncat(temp_buf, tok_3, strlen(tok_3));
  strncat(temp_buf, tok_1, strlen(tok_1));
  strncat(temp_buf, "_log.txt", 9);
  strlcpy(newfilepath, temp_buf, strlen(temp_buf) + 1);

  return newfilepath;
}

/* When the number of files exceeds 10, Delete oldest files. */
static int file_delete(char *file_delete_path, const char *path) {
  const char *dirpath = path;
  char temp_buf[50] = { 0 };
  uint8_t pathlen;

  strncat(temp_buf, dirpath, strlen(dirpath) + 1);
  strncat(temp_buf, "/", 2);
  strncat(temp_buf, file_delete_path, strlen(file_delete_path));

  pathlen = strlen(temp_buf);
  strlcpy(temp_buf, temp_buf, pathlen + 1);

  return unlink(temp_buf);
}

/*The system file log is saved in the file storage device using the "FLOG" macro.*/
int file_log_write(char *format, ...) {
  char *newfilepath;
  int ret = 0;
  char buff[FILE_LOG_MAX_MSG_SIZE] = { 0 };

  va_list list;
  va_start(list, format);
  vsnprintf(buff, sizeof(buff), format, list);
  va_end(list);

  ret = file_status_check(BASE_PATH);
  if (ret != 0) {
    LOGE(TAG, "file status check fail.");
    return -1;
  }

  if (file_ctx.file_num == 0) {
    newfilepath = file_path_name(BASE_PATH);
    write_log(newfilepath, buff);
    free(newfilepath);
    goto END;
  }

  if (file_ctx.latest_file_size >= FILE_LOG_MAX_FILE_SIZE) {
    file_ctx.latest_file_size = 0;
    newfilepath = file_path_name(BASE_PATH);
    write_log(newfilepath, buff);
    free(newfilepath);
  } else {
    write_log(file_ctx.filepath, buff);
  }

  if (file_ctx.file_num > g_file_log_num) {
    if (file_delete(file_ctx.oldest_file, BASE_PATH) != 0) {
      LOGE(TAG, "file delete erorr!");
    }
  }
END:
  return 0;
}

/*The system file log is saved in the file storage device using the "FLOG" macro.*/
int file_log_write_datalogger(char *path, char *format, ...) {
  char *newfilepath;
  int ret = 0;
  char buff[FILE_LOG_MAX_MSG_SIZE] = { 0 };

  va_list list;
  va_start(list, format);
  vsnprintf(buff, sizeof(buff), format, list);
  va_end(list);

  ret = file_status_check(path);
  if (ret != 0) {
    LOGE(TAG, "file status check fail.");
    return -1;
  }

  LOGE(TAG, " file count : %d", file_ctx.file_num);
  LOGI(TAG, " path : %s", path);
  LOGI(TAG, " buff : %s", buff);

  if (file_ctx.file_num == 0) {
    newfilepath = file_path_name(path);
#if (CONFIG_BS_PLATFORM_GASSENSOR)
    char index_buf[60] = "TIME,NH3,CO,H2S,O2,CH4,S_TEMP,S_MOS,S_EC,A_TEMP,A_MOS\n";
    write_log(newfilepath, index_buf);
#endif
    write_log(newfilepath, buff);
    free(newfilepath);
    goto END;
  }

  if (file_ctx.latest_file_size >= FILE_LOG_MAX_FILE_SIZE) {
    file_ctx.latest_file_size = 0;
    newfilepath = file_path_name(path);
#if (CONFIG_BS_PLATFORM_GASSENSOR)
    char index_buf[60] = "TIME,NH3,CO,H2S,O2,CH4,S_TEMP,S_MOS,S_EC,A_TEMP,A_MOS\n";
    write_log(newfilepath, index_buf);
#endif
    write_log(newfilepath, buff);
    free(newfilepath);
  } else {
    write_log(file_ctx.filepath, buff);
  }

  if (file_ctx.file_num > g_file_log_num) {
    LOGE(TAG, "file delete!");
    if (file_delete(file_ctx.oldest_file, path) != 0) {
      LOGE(TAG, "file delete erorr!");
    }
  }
END:
  return 0;
}

void set_file_log_number(int file_log_num) {
  g_file_log_num = file_log_num;
}
