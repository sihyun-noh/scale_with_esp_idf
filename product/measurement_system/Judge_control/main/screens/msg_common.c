
#include "../ui.h"
#include "log.h"
#include <math.h>

/*
extern custom_msg_box_t *user_data_obj;
extern void Msg_Box_No_Btn_e_handler(lv_event_t *e);
extern void memory_allocation_manger();
extern void create_custom_msg_box(const char *msg_text,l v_obj_t *active_screen, void (*event_handler)(lv_event_t *),
lv_event_code_t event);
*/
static const char *TAG = "msg_box_coustom";

custom_msg_box_t *user_data_obj = NULL;
void Msg_Box_No_Btn_e_handler(lv_event_t *e);
void memory_allocation_manger();
void create_custom_msg_box(const char *msg_text, lv_obj_t *active_screen, void (*event_handler)(lv_event_t *),
                           lv_event_code_t event);

void Msg_Box_No_Btn_e_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  // custom_msg_box_t *user_data_obj = lv_event_get_user_data(e);
  if (code == LV_EVENT_CLICKED) {
    LOGI(TAG, "click btn Msg_Box_No_Btn_e_handler!!");
    memory_allocation_manger();
  }
}

void memory_allocation_manger() {
  if (user_data_obj != NULL) {
    // LOGE(TAG, "free(user_data_obj)");
    if (user_data_obj->target1 != NULL) {
      // LOGE(TAG, "lv_obj_del(user_data_obj->target1)");
      lv_obj_del(user_data_obj->target1);
    }
    free(user_data_obj);
    user_data_obj = NULL;  // Setting the pointer to NULL to prevent dangling pointer
  } else {
    // LOGE(TAG, "Memory allocation");
    user_data_obj = (custom_msg_box_t *)malloc(sizeof(custom_msg_box_t));
    if (user_data_obj == NULL) {
      LOGI(TAG, "Memory allocation failure");
    }
  }
}
void create_custom_msg_box(const char *msg_text, lv_obj_t *active_screen, void (*event_handler)(lv_event_t *),
                           lv_event_code_t event) {
  memory_allocation_manger();

  if (user_data_obj == NULL) {
    // LOGE(TAG, "Memory allocation in create custom mag box");
    user_data_obj = (custom_msg_box_t *)malloc(sizeof(custom_msg_box_t));
  }

  lv_obj_t *Msg_Box_Panel = lv_obj_create(active_screen);
  user_data_obj->target1 = Msg_Box_Panel;

  lv_obj_set_width(Msg_Box_Panel, 300);
  lv_obj_set_height(Msg_Box_Panel, 150);
  lv_obj_set_x(Msg_Box_Panel, 0);
  lv_obj_set_y(Msg_Box_Panel, 0);
  lv_obj_set_align(Msg_Box_Panel, LV_ALIGN_CENTER);
  // lv_obj_add_flag(Msg_Box_Panel, LV_OBJ_FLAG_HIDDEN);        /// Flags
  lv_obj_clear_flag(Msg_Box_Panel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(Msg_Box_Panel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(Msg_Box_Panel, lv_color_hex(0xE8BABA), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(Msg_Box_Panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *Msg_Box_Panel_Label = lv_label_create(Msg_Box_Panel);
  user_data_obj->target2 = Msg_Box_Panel_Label;

  lv_obj_set_width(Msg_Box_Panel_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(Msg_Box_Panel_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(Msg_Box_Panel_Label, 0);
  lv_obj_set_y(Msg_Box_Panel_Label, 0);
  lv_label_set_text(Msg_Box_Panel_Label, msg_text);
  lv_obj_set_style_text_font(Msg_Box_Panel_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  if (event_handler != NULL) {
    lv_obj_t *Msg_Box_Yes_Btn = lv_btn_create(Msg_Box_Panel);
    lv_obj_set_width(Msg_Box_Yes_Btn, 80);
    lv_obj_set_height(Msg_Box_Yes_Btn, 40);
    lv_obj_set_x(Msg_Box_Yes_Btn, -50);
    lv_obj_set_y(Msg_Box_Yes_Btn, 35);
    lv_obj_set_align(Msg_Box_Yes_Btn, LV_ALIGN_CENTER);
    lv_obj_add_flag(Msg_Box_Yes_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
    lv_obj_clear_flag(Msg_Box_Yes_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
    // lv_obj_add_event_cb(Msg_Box_Yes_Btn, Msg_Box_Yes_Btn_e_handler, LV_EVENT_CLICKED, user_data_obj);
    lv_obj_add_event_cb(Msg_Box_Yes_Btn, event_handler, event, NULL);

    lv_obj_t *Msg_Box_Yes_Btn_Label = lv_label_create(Msg_Box_Yes_Btn);
    lv_obj_set_width(Msg_Box_Yes_Btn_Label, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(Msg_Box_Yes_Btn_Label, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_x(Msg_Box_Yes_Btn_Label, 10);
    lv_obj_set_y(Msg_Box_Yes_Btn_Label, 0);
    lv_label_set_text(Msg_Box_Yes_Btn_Label, "예");
    lv_obj_set_style_text_font(Msg_Box_Yes_Btn_Label, &NanumBar18, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *Msg_Box_No_Btn = lv_btn_create(Msg_Box_Panel);
    lv_obj_set_width(Msg_Box_No_Btn, 80);
    lv_obj_set_height(Msg_Box_No_Btn, 40);
    lv_obj_set_x(Msg_Box_No_Btn, 50);
    lv_obj_set_y(Msg_Box_No_Btn, 35);
    lv_obj_set_align(Msg_Box_No_Btn, LV_ALIGN_CENTER);
    lv_obj_add_flag(Msg_Box_No_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
    lv_obj_clear_flag(Msg_Box_No_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
    lv_obj_add_event_cb(Msg_Box_No_Btn, Msg_Box_No_Btn_e_handler, event, NULL);

    lv_obj_t *Msg_Box_No_Btn_Label = lv_label_create(Msg_Box_No_Btn);
    lv_obj_set_width(Msg_Box_No_Btn_Label, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(Msg_Box_No_Btn_Label, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_x(Msg_Box_No_Btn_Label, 0);
    lv_obj_set_y(Msg_Box_No_Btn_Label, 0);
    lv_label_set_text(Msg_Box_No_Btn_Label, "아니오");
    lv_obj_set_style_text_font(Msg_Box_No_Btn_Label, &NanumBar18, LV_PART_MAIN | LV_STATE_DEFAULT);

  } else {
    lv_obj_t *Msg_Box_No_Btn = lv_btn_create(Msg_Box_Panel);
    lv_obj_set_width(Msg_Box_No_Btn, 80);
    lv_obj_set_height(Msg_Box_No_Btn, 40);
    lv_obj_set_x(Msg_Box_No_Btn, 50);
    lv_obj_set_y(Msg_Box_No_Btn, 35);
    lv_obj_set_align(Msg_Box_No_Btn, LV_ALIGN_CENTER);
    lv_obj_add_flag(Msg_Box_No_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
    lv_obj_clear_flag(Msg_Box_No_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
    lv_obj_add_event_cb(Msg_Box_No_Btn, Msg_Box_No_Btn_e_handler, event, NULL);

    lv_obj_t *Msg_Box_No_Btn_Label = lv_label_create(Msg_Box_No_Btn);
    lv_obj_set_width(Msg_Box_No_Btn_Label, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(Msg_Box_No_Btn_Label, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_x(Msg_Box_No_Btn_Label, 0);
    lv_obj_set_y(Msg_Box_No_Btn_Label, 0);
    lv_label_set_text(Msg_Box_No_Btn_Label, " 닫힘");
    lv_obj_set_style_text_font(Msg_Box_No_Btn_Label, &NanumBar18, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}
