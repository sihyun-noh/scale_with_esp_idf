#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

int uint16_compare(const void* first, const void* second);

uint16_t uint16_average(uint16_t* array, int size);

int float_compare(const void* first, const void* second);

float float_average(float* array, int size);

#ifdef __cplusplus
}
#endif

#endif /* _UTILS_H_ */
