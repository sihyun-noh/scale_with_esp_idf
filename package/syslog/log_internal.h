#ifndef _LOG_INTERNAL_H_
#define _LOG_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

void log_buffer_hex_internal(const char *tag, const void *buffer, uint16_t buff_len, log_level_t level);
void log_buffer_char_internal(const char *tag, const void *buffer, uint16_t buff_len, log_level_t level);
void log_buffer_hexdump_internal(const char *tag, const void *buffer, uint16_t buff_len, log_level_t level);

#ifdef __cplusplus
}
#endif

#endif /* _LOG_INTERNAL_H_ */
