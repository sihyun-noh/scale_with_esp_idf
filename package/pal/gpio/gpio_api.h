#ifndef _GPIO_API_H_
#define _GPIO_API_H_

#include "stddef.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Initialize the given GPIO bus
 *
 * The bus MUST not be acquired before initializing it, as this is handled
 * internally by the gpio_init function!
 *
 * @param[in]  pin          GPIO device port number to initialize
 * @param[out] mode         GPIO mode (INPUT / OUTPUT)
 */
int gpio_init(uint8_t pin, uint8_t mode);

/**
 * @brief   Convenience function for reading gpio status from a device
 *
 * @param[in]  pin          GPIO peripheral device
 *
 * @return                  0 When success otherwise negative
 */

int gpio_read(uint8_t pin);

/**
 * @brief   Convenience function for reading gpio status from a device
 *
 * @param[in]  pin          GPIO peripheral device
 * @param[out] val          value to write
 *
 * @return                  0 When success otherwise negative
 */

int gpio_write(uint8_t pin, uint8_t val);

#ifdef __cplusplus
}
#endif

#endif /* _GPIO_API_H_ */
