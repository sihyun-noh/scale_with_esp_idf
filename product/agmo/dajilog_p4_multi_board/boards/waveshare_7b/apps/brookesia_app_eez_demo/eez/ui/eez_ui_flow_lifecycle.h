#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/** Stop EEZ Flow and release its runtime state before Brookesia recycles LVGL objects. */
void eez_ui_flow_stop(void);

#ifdef __cplusplus
}
#endif
