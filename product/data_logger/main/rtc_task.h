#ifndef _TIME_TASK_H_
#define _TIME_TASK_H_
#include <time.h>

void rtc_time_init(void);
void rtc_set_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
void rtc_get_time(struct tm* time);
int rtc_set_time_cmd(int argc, char** argv);
int rtc_get_time_cmd(int argc, char** argv);

#endif /* _TIME_TASK_H_ */
