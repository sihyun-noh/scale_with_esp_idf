#include "utils.h"

/**
 * @brief Implement the ascending comparison function
 *
 * @param first The first argument to be used for comparison
 * @param second The second argument to be used for comparison
 *
 * @return comparison value
 */
int uint16_compare(const void* first, const void* second) {
  if (*(uint16_t*)first > *(uint16_t*)second)
    return 1;
  else if (*(uint16_t*)first < *(uint16_t*)second)
    return -1;
  else
    return 0;
}

int float_compare(const void* first, const void* second) {
  if (*(float*)first > *(float*)second)
    return 1;
  else if (*(float*)first < *(float*)second)
    return -1;
  else
    return 0;
}

/**
 * @brief Average calculation
 *
 * @param array array pointer variable
 * @param size average size
 *
 * @return percent value
 */
uint16_t uint16_average(uint16_t* array, int size) {
  uint16_t sum = 0;
  for (int i = 0; i < size; i++)
    sum += array[i];
  return sum / size;
}

float float_average(float* array, int size) {
  float sum = 0.0;
  for (int i = 0; i < size; i++)
    sum += array[i];
  return sum / size;
}

