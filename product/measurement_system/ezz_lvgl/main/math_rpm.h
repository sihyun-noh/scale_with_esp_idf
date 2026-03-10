
#ifndef _MATH_RPM_H_
#define _MATH_RPM_H_
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool voltage_for_rpm_avg(float target_rpm, float *out_v);
#ifdef __cplusplus
}  // extern "C"
#endif
#endif
