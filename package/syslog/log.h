/**
 * @file log.h
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
 */

#ifndef _LOG_H_
#define _LOG_H_

#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Log level
 *
 */
typedef enum {
  LOG_NONE,    /*!< No log output or PVT log */
  LOG_ERROR,   /*!< Critical  errors, software module can not recover on its own */
  LOG_WARN,    /*!< Error condition from which recovery measures have been taken */
  LOG_INFO,    /*!< Information message which describes normal flows of actions or events */
  LOG_DEBUG,   /*!< Extra Information for debugging purpose */
  LOG_VERBOSE, /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output.
                */
} log_level_t;

#ifndef LOG_DEFAULT_LEVEL
#define LOG_DEFAULT_LEVEL LOG_INFO
#endif

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL LOG_INFO
#endif

/**
 * @brief Function which returns timestamp to be used in log output
 *
 */
char *log_timestamp(void);

/**
 * @brief Write message into the log
 *
 * This function is not intended to be used directly.
 * Instead, use one of LOGE, LOGW, LOGI, LOGD, LOGV macros.
 *
 * This function or these macros should not be used from an interrupt.
 *
 */
void log_write(log_level_t level, const char *tag, const char *format, ...);

#include "log_internal.h"

/**
 * @brief Log a buufer of hex bytes at specified level, separated into 16 bytes each line.
 *
 * @param tag       description tag
 * @param buffer    pointer to the buffer array
 * @param buff_len  length of buffer string
 * @param level     level of the log
 *
 */
#define LOG_BUFFER_HEX_LEVEL(tag, buffer, buff_len, level)   \
  do {                                                       \
    if (LOG_LOCAL_LEVEL >= (level)) {                        \
      log_buffer_hex_internal(tag, buffer, buff_len, level); \
    }                                                        \
  } while (0)

/**
 * @breif Log a buffer of characters at specified level.
 *
 * @param tag       description tag
 * @param buffer    pointer to the buffer array
 * @param buff_len  length of buffer string
 * @param level     level of the log
 */
#define LOG_BUFFER_CHAR_LEVEL(tag, buffer, buff_len, level)   \
  do {                                                        \
    if (LOG_LOCAL_LEVEL >= (level)) {                         \
      log_buffer_char_internal(tag, buffer, buff_len, level); \
    }                                                         \
  } while (0)

/**
 * @brief Dump a buffer to the log at specified level.
 *
 * @param tag       description tag
 * @param buffer    pointer to the buffer array
 * @param buff_len  length of buffer string
 * @param level     level of the log
 */
#define LOG_BUFFER_HEXDUMP(tag, buffer, buff_len, level)         \
  do {                                                           \
    if (LOG_LOCAL_LEVEL >= (level)) {                            \
      log_buffer_hexdump_internal(tag, buffer, buff_len, level); \
    }                                                            \
  } while (0)

/**
 * @brief Log a buffer of hex bytes at Info level
 *
 * @param  tag      description tag
 * @param  buffer   Pointer to the buffer array
 * @param  buff_len length of buffer string
 *
 */
#define LOG_BUFFER_HEX(tag, buffer, buff_len)                \
  do {                                                       \
    if (LOG_LOCAL_LEVEL >= LOG_INFO) {                       \
      LOG_BUFFER_HEX_LEVEL(tag, buffer, buff_len, LOG_INFO); \
    }                                                        \
  } while (0)

/**
 * @brief Log a buffer of characters at Info level.
 * Buffer should contain only printable characters.
 *
 * @param  tag      description tag
 * @param  buffer   Pointer to the buffer array
 * @param  buff_len length of buffer string
 *
 */
#define LOG_BUFFER_CHAR(tag, buffer, buff_len)                \
  do {                                                        \
    if (LOG_LOCAL_LEVEL >= LOG_INFO) {                        \
      LOG_BUFFER_CHAR_LEVEL(tag, buffer, buff_len, LOG_INFO); \
    }                                                         \
  } while (0)

#define SLOG_COLOR_BLACK "30"
#define SLOG_COLOR_RED "31"
#define SLOG_COLOR_GREEN "32"
#define SLOG_COLOR_BROWN "33"
#define SLOG_COLOR_BLUE "34"
#define SLOG_COLOR_PURPLE "35"
#define SLOG_COLOR_CYAN "36"
#define SLOG_COLOR(COLOR) "\033[0;" COLOR "m"
#define SLOG_BOLD(COLOR) "\033[1;" COLOR "m"
#define SLOG_RESET_COLOR "\033[0m"
#define SLOG_COLOR_E SLOG_COLOR(SLOG_COLOR_RED)
#define SLOG_COLOR_W SLOG_COLOR(SLOG_COLOR_BROWN)
#define SLOG_COLOR_I SLOG_COLOR(SLOG_COLOR_GREEN)
#define SLOG_COLOR_D
#define SLOG_COLOR_V
#define SLOG_COLOR_P SLOG_COLOR(SLOG_COLOR_GREEN)

#define SLOG_FORMAT(letter, format) SLOG_COLOR_##letter #letter " (%s) %s: " format SLOG_RESET_COLOR "\r\n"

#define LOGP(tag, format, ...) LOG_LOCAL_DISP_LEVEL(LOG_NONE, tag, format, ##__VA_ARGS__)
#define LOGE(tag, format, ...) LOG_LOCAL_DISP_LEVEL(LOG_ERROR, tag, format, ##__VA_ARGS__)
#define LOGW(tag, format, ...) LOG_LOCAL_DISP_LEVEL(LOG_WARN, tag, format, ##__VA_ARGS__)
#define LOGI(tag, format, ...) LOG_LOCAL_DISP_LEVEL(LOG_INFO, tag, format, ##__VA_ARGS__)
#define LOGD(tag, format, ...) LOG_LOCAL_DISP_LEVEL(LOG_DEBUG, tag, format, ##__VA_ARGS__)
#define LOGV(tag, format, ...) LOG_LOCAL_DISP_LEVEL(LOG_VERBOSE, tag, format, ##__VA_ARGS__)

/**
 * @brief runtime macro to output logs at a specified level.
 *
 * @param tag tag of the log, which can be used to change the log level by ``log_level_set`` at runtime.
 * @param level level of the output log.
 * @param format format of the output log. see ``printf``
 * @param ... variables to be replaced into the log. see ``printf``
 *
 * @see ``printf``
 */
#define LOG_DISP_LEVEL(level, tag, format, ...)                                                 \
  do {                                                                                          \
    if (level == LOG_ERROR) {                                                                   \
      log_write(LOG_ERROR, tag, SLOG_FORMAT(E, format), log_timestamp(), tag, ##__VA_ARGS__);   \
    } else if (level == LOG_WARN) {                                                             \
      log_write(LOG_WARN, tag, SLOG_FORMAT(W, format), log_timestamp(), tag, ##__VA_ARGS__);    \
    } else if (level == LOG_DEBUG) {                                                            \
      log_write(LOG_DEBUG, tag, SLOG_FORMAT(D, format), log_timestamp(), tag, ##__VA_ARGS__);   \
    } else if (level == LOG_VERBOSE) {                                                          \
      log_write(LOG_VERBOSE, tag, SLOG_FORMAT(V, format), log_timestamp(), tag, ##__VA_ARGS__); \
    } else if (level == LOG_INFO) {                                                             \
      log_write(LOG_INFO, tag, SLOG_FORMAT(I, format), log_timestamp(), tag, ##__VA_ARGS__);    \
    } else {                                                                                    \
      log_write(LOG_NONE, tag, SLOG_FORMAT(P, format), log_timestamp(), tag, ##__VA_ARGS__);    \
    }                                                                                           \
  } while (0)

/**
 * @brief runtime macro to output logs at a specified level.
 * Also check the level with ``LOG_LOCAL_DISP_LEVEL``.
 *
 * @see ``printf``, ``LOG_DEFAULT_LEVEL``
 */
#define LOG_LOCAL_DISP_LEVEL(level, tag, format, ...)    \
  do {                                                   \
    if (level <= LOG_DEFAULT_LEVEL)                      \
      LOG_DISP_LEVEL(level, tag, format, ##__VA_ARGS__); \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif /* _LOG_H_ */
