/**
 * @file icmp_echo_api.h
 * 
 * @brief  It measures the round-trip time for messages sent from the source host to a destination target that are
 * echoed back to the source.
 * 
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "esp32/icmp_echo_impl.h"

/**
 * @brief Ping operates by sending Internet Control Message Protocol (ICMP) echo request packets to the target host and
 * waiting for an ICMP echo reply.
 * 
 * @param host target host(e.g. www.google.com)
 * @param ping_count A"ping" session count
 * @return int success return "PING_OK", fail return "PING_ERR_UNKNOWN_HOST"
 */
int do_ping(char *host, uint8_t ping_count);