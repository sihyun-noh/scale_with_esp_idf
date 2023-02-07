/**
 * @file syslog.c
 *
 * @brief This api is designed to be save the log message to the flash and send it to the cloud server.
 * It is implemented in a way that manages log messages in a list structure using a 64kb buffer and
 * deletes the previous log when the buffer size is exceeded.
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "syslog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

static const char *TAG = "syslog";

#define dbg(format, args...) LOGD(TAG, format, ##args)
#define dbgw(format, args...) LOGW(TAG, format, ##args)
#define dbge(format, args...) LOGE(TAG, format, ##args)

#define SYSLOG_MAX_BUFF_SIZE 1024
#define SYSLOG_TIMESTAMP_SIZE 128
#define SYSLOG_MAX_MSG_SIZE (SYSLOG_MAX_BUFF_SIZE)
#define SYSLOG_SIZE (SYSLOG_MAX_BUFF_SIZE * 32)

#if (SYSLOG_MAX_BUFF_SIZE > SYSLOG_SIZE)
#error "SYSLOG_MAX_BUFF_SIZE must be less than SYSLOG_SIZE"
#endif

SemaphoreHandle_t syslog_mutex;
portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

typedef struct syslog {
  uint32_t num;
  char *message;
  struct syslog *prev;
  struct syslog *next;
} syslog_t;

typedef struct _syslog_list {
  syslog_t *head;
  syslog_t *tail;
  uint32_t len;  // Includes terminating NULL
} syslog_list_t;

static syslog_list_t syslog_list;
static uint8_t syslog_dump_buff[SYSLOG_SIZE];

int cloud_log(char *message) {
  // TODO: Implement this function to send the log message to the cloud server.
  // Until this function is implemented, the log message will be printed to the console to check the result.
  printf("%s\n", message);
  return 0;
}

void syslog_init(void) {
  syslog_mutex = xSemaphoreCreateMutex();
  xSemaphoreGive(syslog_mutex);
}

static syslog_t *create_syslog(const char *message) {
  static uint32_t lognum = 0;

  syslog_t *item;

  char *syslog_msg;
  uint32_t syslog_msg_len;
  int msg_len = strlen(message);

  if (message == NULL || msg_len == 0 || msg_len > SYSLOG_MAX_MSG_SIZE - 1) {
    return NULL;
  }

  // Generate syslog message
  syslog_msg_len = strlen(message) + 1;
  syslog_msg = (char *)malloc(syslog_msg_len);
  memset(syslog_msg, 0, syslog_msg_len);
  memcpy(syslog_msg, message, syslog_msg_len);

  // Create syslog data
  item = (syslog_t *)malloc(sizeof(syslog_t));
  if (item == NULL) {
    free(syslog_msg);
    return NULL;
  }

  memset(item, 0, sizeof(syslog_t));
  item->num = lognum;
  item->message = syslog_msg;
  item->prev = NULL;
  item->next = NULL;

  portENTER_CRITICAL(&spinlock);
  ++lognum;
  portEXIT_CRITICAL(&spinlock);

  return item;
}

// Add syslog message to the list
static void add_to_list(syslog_list_t *list, syslog_t *item) {
  if (item == NULL) {
    return;
  }
  if (list->head == NULL && list->tail == NULL) {
    list->head = item;
    list->tail = item;
    list->len += (strlen(item->message) + 1);
    return;
  }

  item->prev = list->tail;
  item->next = NULL;

  list->tail->next = item;
  list->tail = item;
  list->len += (strlen(item->message) + 1);

  return;
}

// Remove syslog message from the list
static int del_from_list(syslog_list_t *list) {
  syslog_t *tmp;

  if (list->head == NULL || list->tail == NULL) {
    return -1;
  }

  tmp = list->head;
  if (list->head == list->tail) {
    list->head = NULL;
    list->tail = NULL;
  } else {
    list->head = list->head->next;
    list->head->prev = NULL;
  }
  list->len -= (strlen(tmp->message) + 1);

  free(tmp->message);
  free(tmp);

  return 0;
}

// Debug syslog message
static void _dbg_list(syslog_list_t *list) {
  syslog_t *log;
  if (list->head == NULL || list->tail == NULL) {
    dbg("syslog list is empty");
    return;
  }
  dbg("syslog list: num (%d) len (%d)", list->tail->num, list->len);
  log = list->head;
  do {
    char *mark;
    if (log == list->head && log == list->tail) {
      mark = (char *)"<-- head, tail";
    } else if (log == list->head) {
      mark = (char *)"<-- head";
    } else if (log == list->tail) {
      mark = (char *)"<-- tail";
    } else {
      mark = (char *)"";
    }
    printf("%.64s (%ld) %s\n", log->message, log->num, mark);
    log = log->next;
  } while (log);
}

void dbg_syslog(void) {
  _dbg_list(&syslog_list);
}

// Dump syslog messages to the buffer
char *get_dump_syslog(void) {
  syslog_list_t *list = &syslog_list;
  syslog_t *log;
  char *dump_buffer;

  if (syslog_mutex == NULL) {
    return NULL;
  }

  if (xSemaphoreTake(syslog_mutex, pdMS_TO_TICKS(10 * 1000)) != pdTRUE) {
    dbg("get_dump_syslog: failed to take syslog mutex");
    return NULL;
  }

  if (list->head == NULL || list->tail == NULL) {
    dbg("get_dump_syslog: syslog list is empty");
    xSemaphoreGive(syslog_mutex);
    return NULL;
  }

  // Get reference memory for dump buffer to capture syslog messages
  dump_buffer = (char *)syslog_dump_buff;
  memset(dump_buffer, 0, sizeof(syslog_dump_buff));

  // Dump syslog messages to the buffer
  log = list->head;
  do {
    strcat(dump_buffer, log->message);
    strcat(dump_buffer, "\n");
    log = log->next;
  } while (log && log != list->tail);

  xSemaphoreGive(syslog_mutex);
  return dump_buffer;
}

// Publish log message to cloud logging server
void publish_syslog(void) {
  char buff[SYSLOG_MAX_BUFF_SIZE];
  uint32_t offset = 0;
  char *sys_msg, *nl;
  uint32_t smsg_len = 0, line_len = 0;

  cloud_log((char *)"-------------- BEGIN OF SYSLOG DUMP --------------");
  sys_msg = get_dump_syslog();
  while (1) {
    if (sys_msg == NULL) {
      break;
    }
    // If sys_msg is less than SYSLOG_MAX_BUFF_SIZE, send it directly and then exit while loop, because it is last
    // message
    smsg_len = strlen(sys_msg);
    if (smsg_len < sizeof(buff) - 1) {
      memset(buff, 0, sizeof(buff));
      memcpy(buff, sys_msg, sizeof(buff) - 1);
      cloud_log(buff);
      break;
    }

    // If sys_msg is more than SYSLOG_MAX_BUFF_SIZE, send it as chunk data(1024 bytes) and then continue to next chunk
    // If the last data is less than 1024 bytes, it is checked in the if statement above.
    // sys_msg's buffer point is moved to the next chunk
    memset(buff, 0, sizeof(buff));
    offset = 0;

    while (1) {
      nl = strchr(sys_msg, '\n');
      line_len = nl - sys_msg + 1;
      if (offset + line_len > sizeof(buff) - 1) {
        break;
      }
      memcpy(buff + offset, sys_msg, line_len);
      offset += line_len;
      sys_msg += line_len;
    }

    cloud_log(buff);
  }
  cloud_log((char *)"-------------- END OF SYSLOG DUMP --------------");
}

int _syslog(const char *message) {
  syslog_t *item;
  uint32_t remaining;
  uint32_t required;

  if (syslog_mutex == NULL) {
    return -1;
  }
  if (message == NULL) {
    return -1;
  }

  if (xSemaphoreTake(syslog_mutex, pdMS_TO_TICKS(3 * 1000)) != pdTRUE) {
    dbg("_syslog: failed to take syslog mutex");
    return -1;
  }

  item = create_syslog(message);
  if (item == NULL) {
    xSemaphoreGive(syslog_mutex);
    return -1;
  }

  remaining = SYSLOG_SIZE - syslog_list.len;
  required = strlen(item->message) + 1;

  while (remaining < required) {
    if (del_from_list(&syslog_list) != 0) {
      break;
    }
    remaining = SYSLOG_SIZE - syslog_list.len;
  }

  add_to_list(&syslog_list, item);

  xSemaphoreGive(syslog_mutex);
  return 0;
}

int syslog(const char *format, ...) {
  int ret;
  char buff[SYSLOG_MAX_MSG_SIZE] = {
    0,
  };

  va_list list;
  va_start(list, format);
  vsnprintf(buff, sizeof(buff), format, list);
  va_end(list);

  if (!(ret = _syslog(buff))) {
    cloud_log(buff);
  }

  return ret;
}
