#include "../ui.h"
#include "log.h"

static const char *TAG = "ui_judge_color_Screen";

void ui_Judge_Color_Screen_Panel_e_handler(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  if (event_code == LV_EVENT_CANCEL) {
    _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen1_screen_init);
  }
}

static void anim_x_cb(void *var, int32_t v) {
  lv_obj_set_x(var, v);
}

static void anim_size_cb(void *var, int32_t v) {
  lv_obj_set_size(var, v, v);
}

void ui_judge_color_screen_init(void) {
  ui_judge_color_Screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_judge_color_Screen, LV_OBJ_FLAG_SCROLLABLE);  /// Flags

  ui_judge_color_ScreenPanel = lv_obj_create(ui_judge_color_Screen);
  lv_obj_set_width(ui_judge_color_ScreenPanel, 350);
  lv_obj_set_height(ui_judge_color_ScreenPanel, 260);
  lv_obj_set_x(ui_judge_color_ScreenPanel, 0);
  lv_obj_set_y(ui_judge_color_ScreenPanel, 1);
  lv_obj_set_align(ui_judge_color_ScreenPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_judge_color_ScreenPanel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_bg_color(ui_judge_color_ScreenPanel, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN);

  lv_obj_add_event_cb(ui_judge_color_ScreenPanel, ui_Judge_Color_Screen_Panel_e_handler, LV_EVENT_ALL, NULL);

  lv_anim_t anim;
  lv_anim_init(&anim);
  lv_anim_set_var(&anim, ui_judge_color_ScreenPanel);
  lv_anim_set_values(&anim, 290, 300);
  lv_anim_set_time(&anim, 1000);
  lv_anim_set_playback_delay(&anim, 100);
  lv_anim_set_playback_time(&anim, 300);
  lv_anim_set_repeat_delay(&anim, 500);
  lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
  lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);

  lv_anim_set_exec_cb(&anim, anim_size_cb);
  lv_anim_start(&anim);
  lv_anim_set_exec_cb(&anim, anim_x_cb);
  lv_anim_set_values(&anim, 0, 0);
  lv_anim_start(&anim);
}
