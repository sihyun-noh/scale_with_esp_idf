#ifndef _UI_MSGEVT_H_
#define _UI_MSGEVT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
  UI_CMD_NONE,
  // ---- UI -> HW ----
  UI_CMD_DRIVER1_SET,
  UI_CMD_DRIVER2_SET,
  UI_CMD_BOTH_SET_RPM,
  UI_CMD_EVT_BOTH,
  UI_CMD_RUN_STATE,

  // ---- HW -> UI ----
  UI_EVT_FB_DRIVER1_R,
  UI_EVT_FB_DRIVER1_L,
  UI_EVT_FB_DRIVER2_R,
  UI_EVT_FB_DRIVER2_L,

} ui_msg_id_t;

typedef struct {
  ui_msg_id_t id;
  int32_t value;
  char *str;
} ui_msg_t;

#ifdef __cplusplus
}
#endif
#endif
