#ifndef _SYSLOG_H_
#define _SYSLOG_H_

#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void dbg_syslog(void);
extern void publish_syslog(void);
extern char *get_dump_syslog(void);
extern int syslog(const char *format, ...);
extern void syslog_init(void);

#define SYSLOG_FORMAT(letter, format) #letter " (%s) %s: " format "\r\n"

#define SYSLOGI(tag, format, ...) SYS_LOG_LEVEL(LOG_INFO, tag, format, ##__VA_ARGS__)
#define SYSLOGW(tag, format, ...) SYS_LOG_LEVEL(LOG_WARN, tag, format, ##__VA_ARGS__)
#define SYSLOGE(tag, format, ...) SYS_LOG_LEVEL(LOG_ERROR, tag, format, ##__VA_ARGS__)

#define SYS_LOG_LEVEL(level, tag, format, ...)                               \
  do {                                                                       \
    if (level == LOG_ERROR) {                                                \
      syslog(SYSLOG_FORMAT(E, format), log_timestamp(), tag, ##__VA_ARGS__); \
    } else if (level == LOG_WARN) {                                          \
      syslog(SYSLOG_FORMAT(W, format), log_timestamp(), tag, ##__VA_ARGS__); \
    } else {                                                                 \
      syslog(SYSLOG_FORMAT(I, format), log_timestamp(), tag, ##__VA_ARGS__); \
    }                                                                        \
  } while (0)

#define SLOGI(tag, format, ...)          \
  do {                                   \
    SYSLOGI(tag, format, ##__VA_ARGS__); \
    LOGI(tag, format, ##__VA_ARGS__);    \
  } while (0)

#define SLOGW(tag, format, ...)          \
  do {                                   \
    SYSLOGW(tag, format, ##__VA_ARGS__); \
    LOGW(tag, format, ##__VA_ARGS__);    \
  } while (0)

#define SLOGE(tag, format, ...)          \
  do {                                   \
    SYSLOGE(tag, format, ##__VA_ARGS__); \
    LOGE(tag, format, ##__VA_ARGS__);    \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif /* _SYSLOG_H_ */
