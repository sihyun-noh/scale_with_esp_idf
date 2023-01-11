#ifndef _BATTERY_TASK_H_
#define _BATTERY_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif

void battery_init(void);
int read_battery_percentage(void);

#ifdef __cplusplus
}
#endif

#endif /* _BATTERY_TASK_H_ */
