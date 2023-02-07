/**
 * @file mdns.c
 *
 * @brief MDNS-SD Query and advertise
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include "log.h"
#include "mdns.h"
#include "syscfg.h"
#include "esp_mac.h"

#define CONFIG_MDNS_INSTANCE "mDNS SensorNode"

static const char* TAG = "mDNS";
char* generate_hostname(void);

/**
 * @brief initialise mdns service
 *
 */
void initialise_mdns(void) {
  char* hostname = generate_hostname();

  // initialize mDNS
  ESP_ERROR_CHECK(mdns_init());
  // set mDNS hostname (required if you want to advertise services)
  ESP_ERROR_CHECK(mdns_hostname_set(hostname));
  LOGI(TAG, "mdns hostname set to: [%s]", hostname);
  // set default mDNS instance name
  ESP_ERROR_CHECK(mdns_instance_name_set(CONFIG_MDNS_INSTANCE));

  // structure with TXT records
  mdns_txt_item_t serviceTxtData[1] = {
    { "Board", "ESP32" },
  };

  // initialize service
  ESP_ERROR_CHECK(mdns_service_add("greenlabs-mdns", "_http", "_tcp", 80, serviceTxtData, 1));

  free(hostname);
}

/**
 * @brief Generate host name based on sdkconfig, optionally adding a portion of MAC address to it.
 *
 * @return host name string allocated from the heap
 */
char* generate_hostname(void) {
  uint8_t mac[6] = { 0 };
  char model_name[10] = { 0 };
  char* hostname;

  syscfg_get(MFG_DATA, "model_name", model_name, sizeof(model_name));
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  if (-1 ==
      asprintf(&hostname, "%s-%02X%02X%02X%02X%02X%02X", model_name, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5])) {
    abort();
  }
  return hostname;
}
