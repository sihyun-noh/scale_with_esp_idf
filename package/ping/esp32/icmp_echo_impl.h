/**
 * @file icmp_echo_impl.h
 * 
 * @brief Type of ��ping�� callback functions is provided by espressif.
 * 
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "ping/ping_sock.h"

enum {
  PING_OK = 0,
  PING_ERR_NOT_COUNT,
  PING_ERR_UNKNOWN_HOST,
};

/**
 * @brief Invoked by internal ping thread when received ICMP echo reply packet.
 *
 * @param hdl handle of ping session
 * @param args
 */
void cmd_ping_on_ping_success(esp_ping_handle_t hdl, void *args);

/**
 * @brief Invoked by internal ping thread when receive ICMP echo reply packet timeout.
 *
 * @param hdl handle of ping session
 * @param args
 */
void cmd_ping_on_ping_timeout(esp_ping_handle_t hdl, void *args);

/**
 * @brief Invoked by internal ping thread when a ping session is finished.
 *
 * @param hdl handle of ping session
 * @param args
 */
void cmd_ping_on_ping_end(esp_ping_handle_t hdl, void *args);