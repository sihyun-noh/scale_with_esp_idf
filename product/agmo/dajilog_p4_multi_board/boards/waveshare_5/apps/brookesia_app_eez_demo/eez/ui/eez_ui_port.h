#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Start the generated EEZ UI and attach its adaptor/bridge binding. */
bool eez_ui_port_create(void);

/** Detach the binding before Brookesia releases the generated LVGL resources. */
void eez_ui_port_prepare_cleanup(void);

#ifdef __cplusplus
}
#endif
