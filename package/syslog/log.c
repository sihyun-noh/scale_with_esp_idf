/**
 * @file log.c
 *
 * @brief This log api is made inspired by esp-idf logging api.
 * This api is designed to be used in the same way as esp-idf logging api and is used on all platforms.
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 *
 */
#include "log.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <time.h>

// print number of bytes per line for log_buffer_char and log_buffer_hex
#define BYTES_PER_LINE 16
#define LOG_TIMESTAMP_SIZE 64

#define MAX_MUTEX_WAIT_MS 10
#define MAX_MUTEX_WAIT_TICKS ((MAX_MUTEX_WAIT_MS + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS)

static SemaphoreHandle_t s_log_mutex = NULL;
static log_level_t s_log_default_level = LOG_VERBOSE;

void log_lock(void) {
  if (!s_log_mutex) {
    s_log_mutex = xSemaphoreCreateMutex();
  }
  if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
    return;
  }
  xSemaphoreTake(s_log_mutex, portMAX_DELAY);
}

void log_unlock(void) {
  if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
    return;
  }
  xSemaphoreGive(s_log_mutex);
}

bool log_lock_timeout(void) {
  if (!s_log_mutex) {
    s_log_mutex = xSemaphoreCreateMutex();
  }
  if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
    return true;
  }
  return (xSemaphoreTake(s_log_mutex, MAX_MUTEX_WAIT_TICKS) == pdTRUE);
}

static bool should_output(log_level_t level_for_message, log_level_t level_for_tag) {
  return level_for_message <= level_for_tag;
}

char *log_timestamp(void) {
  static char timestamp[LOG_TIMESTAMP_SIZE];
  static _lock_t bufferLock = 0;

  time_t now = time(NULL);
  struct tm *tm = localtime(&now);

  _lock_acquire(&bufferLock);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm);
  _lock_release(&bufferLock);

  return timestamp;
}

void log_write(log_level_t level, const char *tag, const char *format, ...) {
  if (!log_lock_timeout()) {
    return;
  }

  log_level_t level_for_tag = s_log_default_level;

  if (!should_output(level, level_for_tag)) {
    log_unlock();
    return;
  }

  va_list list;
  va_start(list, format);
  vprintf(format, list);
  va_end(list);

  log_unlock();
}

void log_buffer_hex_internal(const char *tag, const void *buffer, uint16_t buff_len, log_level_t log_level) {
  if (buffer == NULL || buff_len == 0) {
    return;
  }

  char *buff = (char *)buffer;
  char hex_buffer[3 * BYTES_PER_LINE + 1];
  const char *ptr_line;
  int bytes_cur_line;

  do {
    if (buff_len > BYTES_PER_LINE) {
      bytes_cur_line = BYTES_PER_LINE;
    } else {
      bytes_cur_line = buff_len;
    }

    ptr_line = buff;

    for (int i = 0; i < bytes_cur_line; i++) {
      sprintf(hex_buffer + 3 * i, "%02x ", ptr_line[i]);
    }

    LOG_DISP_LEVEL(log_level, tag, "%s", hex_buffer);

    buff += bytes_cur_line;
    buff_len -= bytes_cur_line;
  } while (buff_len);
}

void log_buffer_char_internal(const char *tag, const void *buffer, uint16_t buff_len, log_level_t log_level) {
  if (buffer == NULL || buff_len == 0) {
    return;
  }

  char *buff = (char *)buffer;
  char char_buffer[BYTES_PER_LINE + 1];
  const char *ptr_line;
  int bytes_cur_line;

  do {
    if (buff_len > BYTES_PER_LINE) {
      bytes_cur_line = BYTES_PER_LINE;
    } else {
      bytes_cur_line = buff_len;
    }

    ptr_line = buff;

    for (int i = 0; i < bytes_cur_line; i++) {
      sprintf(char_buffer + i, "%c", ptr_line[i]);
    }

    LOG_DISP_LEVEL(log_level, tag, "%s", char_buffer);

    buff += bytes_cur_line;
    buff_len -= bytes_cur_line;
  } while (buff_len);
}

void log_buffer_hexdump_internal(const char *tag, const void *buffer, uint16_t buff_len, log_level_t log_level) {
  if (buffer == NULL || buff_len == 0) {
    return;
  }

  char *buff = (char *)buffer;
  const char *ptr_line;
  // format: field[length]
  //  ADDR[10]+"   "+DATA_HEX[8*3]+" "+DATA_HEX[8*3]+"  |"+DATA_CHAR[8]+"|"
  char hd_buffer[10 + 3 + BYTES_PER_LINE * 3 + 3 + BYTES_PER_LINE + 1 + 1] = {
    0,
  };
  char *ptr_hd;
  int bytes_cur_line;

  do {
    if (buff_len > BYTES_PER_LINE) {
      bytes_cur_line = BYTES_PER_LINE;
    } else {
      bytes_cur_line = buff_len;
    }

    ptr_line = buff;
    ptr_hd = hd_buffer;

    ptr_hd += sprintf(ptr_hd, "%p ", buff);

    for (int i = 0; i < BYTES_PER_LINE; i++) {
      if ((i & 7) == 0) {
        ptr_hd += sprintf(ptr_hd, "%s", " ");
      }
      if (i < bytes_cur_line) {
        ptr_hd += sprintf(ptr_hd, " %02x", ptr_line[i] & 0xff);
      } else {
        ptr_hd += sprintf(ptr_hd, "%s", "   ");
      }
    }

    ptr_hd += sprintf(ptr_hd, "%s", "  |");

    for (int i = 0; i < bytes_cur_line; i++) {
      if (isprint((int)ptr_line[i])) {
        ptr_hd += sprintf(ptr_hd, "%c", ptr_line[i]);
      } else {
        ptr_hd += sprintf(ptr_hd, "%s", ".");
      }
    }

    ptr_hd += sprintf(ptr_hd, "%s", "|");

    LOG_DISP_LEVEL(log_level, tag, "%s", hd_buffer);

    buff += bytes_cur_line;
    buff_len -= bytes_cur_line;
  } while (buff_len);
}
