/**
 * @file shell_console.c
 *
 * @brief Interactive command shell for testing and debugging purposes
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "shell_console.h"

sc_ctx_t *sc_init(void) {
  return sc_init_impl();
}

int sc_start(sc_ctx_t *ctx) {
  return sc_start_impl(ctx);
}

void sc_stop(sc_ctx_t *ctx) {
  sc_stop_impl(ctx);
}
