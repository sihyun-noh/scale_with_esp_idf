#ifndef _DAC_API_H_
#define _DAC_API_H_

#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Initialize the given DAC bus
 *
 * The bus MUST not be acquired before initializing it, as this is handled
 * internally by the dac_init function!
 *
 */
void dac_init(uint8_t chan);

/**
 * @brief   Convenience function for reading dac value from a device
 *
 * @param[in]  pin          DAC peripheral device
 *
 * @return                  0 When success otherwise negative
 */

void dac_write(uint8_t chan, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif /* _DAC_API_H_ */
