/*
 */
#ifndef ETHERNET_INIT_H
#define ETHERNET_INIT_H

#include "esp_eth_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Ethernet driver
 */

void app_eth_init(void);

#ifdef __cplusplus
}
#endif
#endif  // ETHERNET_INIT_H
