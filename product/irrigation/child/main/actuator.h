#ifndef _ACTUATOR_H_
#define _ACTUATOR_H_

typedef enum { INPUT = 0, INPUT_PULLUP = 1, OUTPUT = 2 } gpio_hal_mode_t;

#ifdef __cplusplus
extern "C" {
#endif

int valve_init(void);

void valve_open(void);

void valve_close(void);

#ifdef __cplusplus
}
#endif

#endif /* _ACTUATOR_H_ */
