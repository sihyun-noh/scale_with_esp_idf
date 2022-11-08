/**
 * @file icmp_echo_cmd.h
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

/**
 * @brief Ping operates by sending Internet Control Message Protocol (ICMP) echo request packets to the target host and
 * waiting for an ICMP echo reply.
 *
 * @param argc
 * @param argv
 * @return int
 */

#ifdef __cplusplus
extern "C" {
#endif

int ping_cmd(int argc, char **argv);

#ifdef __cplusplus
}
#endif
