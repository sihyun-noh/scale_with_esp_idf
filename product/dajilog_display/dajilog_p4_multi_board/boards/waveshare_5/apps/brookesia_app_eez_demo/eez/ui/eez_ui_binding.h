#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool eez_ui_binding_attach(void);
void eez_ui_binding_process(void);
void eez_ui_binding_detach(void);

#ifdef __cplusplus
}
#endif
