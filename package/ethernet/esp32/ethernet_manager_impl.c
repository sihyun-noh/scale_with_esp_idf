/**
 * @file ethernet_manager_impl.c
 *
 * @brief ethernet Manager implementation for esp32 platform
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "log.h"
// #include "driver/gpio.h"
// #include "sdkconfig.h"

static const char *TAG = "ethernet_impl";

// /** Event handler for Ethernet events */
// static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
//   uint8_t mac_addr[6] = { 0 };
//   /* we can get the ethernet driver handle from event data */
//   esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

//   switch (event_id) {
//     case ETHERNET_EVENT_CONNECTED:
//       esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
//       ESP_LOGI(TAG, "Ethernet Link Up");
//       ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2],
//                mac_addr[3], mac_addr[4], mac_addr[5]);
//       break;
//     case ETHERNET_EVENT_DISCONNECTED: ESP_LOGI(TAG, "Ethernet Link Down"); break;
//     case ETHERNET_EVENT_START: ESP_LOGI(TAG, "Ethernet Started"); break;
//     case ETHERNET_EVENT_STOP: ESP_LOGI(TAG, "Ethernet Stopped"); break;
//     default: break;
//   }
// }

int ethernet_init_impl(void) {
  // Initialize TCP/IP network interface (should be called only once in application)
  ESP_ERROR_CHECK(esp_netif_init());
  // Create default event loop that running in background
  // ESP_ERROR_CHECK(esp_event_loop_create_default());

  // Create new default instance of esp-netif for Ethernet
  esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
  esp_netif_t *eth_netif = esp_netif_new(&cfg);

  // Init MAC and PHY configs to default
  eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
  eth_esp32_emac_config_t esp32_emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
  eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

  phy_config.phy_addr = 0;
  phy_config.reset_gpio_num = -1;

  esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&esp32_emac_config, &mac_config);
  esp_eth_phy_t *phy = esp_eth_phy_new_lan87xx(&phy_config);
  esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
  esp_eth_handle_t eth_handle = NULL;
  ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
  /* attach Ethernet driver to TCP/IP stack */
  ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));

  // Register user defined event handers
  // ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));

  /* start Ethernet driver state machine */
  ESP_ERROR_CHECK(esp_eth_start(eth_handle));

  LOGI(TAG, "Ethernet Init");

  return 0;
}
