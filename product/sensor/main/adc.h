#ifndef _ADC_H_
#define _ADC_H_

#ifdef __cplusplus
extern "C" {
#endif

void battery_init(void);
int read_battery_percentage(void);

#ifdef __cplusplus
}
#endif

#endif /* _ADC_H_ */
