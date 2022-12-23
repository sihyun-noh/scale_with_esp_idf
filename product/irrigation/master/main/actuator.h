#ifndef _ACTUATOR_H_
#define _ACTUATOR_H_

typedef enum { INPUT = 0, INPUT_PULLUP = 1, OUTPUT = 2 } gpio_hal_mode_t;

#ifdef __cplusplus
extern "C" {
#endif

int pump_init(void);

void pump_on(void);

void pump_off(void);

#ifdef __cplusplus
}
#endif

#endif /* _ACTUATOR_H_ */
