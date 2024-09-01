#include <stdint.h>
#include "../ui.h"
#include "core/lv_event.h"
#include "core/lv_obj.h"
#include "core/lv_obj_scroll.h"
#include "extra/layouts/flex/lv_flex.h"
#include "log.h"
#include "misc/lv_area.h"
#include "syscfg.h"
#include "esp_system.h"
#include "main.h"

#include "../model_define.h"
#include "../scale_read_485.h"

static const char *TAG = "ui_indicator_model_select_screen";

extern custom_msg_box_t *user_data_obj;
extern void Msg_Box_No_Btn_e_handler(lv_event_t *e);
extern void memory_allocation_manger();
extern void create_custom_msg_box(const char *msg_text, lv_obj_t *active_screen, void (*event_handler)(lv_event_t *),
                                  lv_event_code_t event);
extern void create_custom_msg_box_type_1(const char *msg_text, lv_obj_t *active_screen,
                                         void (*yes_event_handler)(lv_event_t *),
                                         void (*no_event_handler)(lv_event_t *), lv_event_code_t event);

lv_obj_t *ui_Indicator_Model_Select_Screen_Comfirm_Btn;
lv_obj_t *ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label;
lv_obj_t *ui_SPK_Set_State_Label;
lv_obj_t *ui_PRT_Set_State_Label;

typedef enum {
  STATE_CHANGE_SPK = 0x01,
  STATE_CHANGE_USB,
} state_change_id_t;

typedef struct state_change {
  state_change_id_t id;
  bool usb_state_change_flag;
  char before_change[5];
  char after_change[5];
} state_change_t;

state_change_t spk_state;
state_change_t usb_state;

typedef struct {
  char *model_series_name;
  char **model_name_list;
  int num_elements;
  uint8_t model_id;         // 모델종류
  uint8_t model_series_id;  // 모델 종류 시리즈
} model_series_data_t;

typedef struct logic_refer_data {
  lv_obj_t *obj_front;
  lv_obj_t *obj_back;
  char series_name[15];
  char model_data[15];
  uint8_t model_id;         // 모델종류
  uint8_t model_series_id;  // 모델 종류 시리즈
} ui_u_data_t;

// 모델 표시 갯수
lv_obj_t *model_btn[10];

// UI 제어 및 최종선택된 모델 확인
ui_u_data_t user_data[10] = { 0 };

// 최종 모델
char final_model_name[10] = { 0 };

void model_series_cancel_btn_e_handler(lv_event_t *e) {
  lv_obj_t *target = lv_event_get_user_data(e);
  lv_event_code_t event_code = lv_event_get_code(e);
  if (event_code == LV_EVENT_CLICKED) {
    lv_obj_del(target);
  }
}

void model_series_btn_e_handler(lv_event_t *e) {
  ui_u_data_t *target = lv_event_get_user_data(e);
  lv_event_code_t event_code = lv_event_get_code(e);
  char str[100] = { 0 };
  char *ptr = final_model_name;
  LOGI(TAG, "receive data %s", target->model_data);
  if (event_code == LV_EVENT_CLICKED) {
    snprintf(str, sizeof(str), "선택된 모델은 \n%s/%s \n입니다.", target->series_name, target->model_data);
    LOGI(TAG, "complated(final) data %s", str);
    // SWII_30_kg_100_g_kg
    memset(ptr, 0x00, sizeof(final_model_name));
    // int text_len = strlen(target->series_name);
    // memcpy(ptr, target->series_name, strlen(target->series_name));
    // memcpy(ptr + text_len, "_", 1);
    // memcpy(ptr + text_len + 1, target->model_data, strlen(target->model_data));
    sprintf(ptr, "%d_%d", target->model_id, target->model_series_id);
    LOGI(TAG, "final save model id : %d", target->model_id);
    LOGI(TAG, "final save model series id : %d", target->model_series_id);
    LOGI(TAG, "final save name : %s", final_model_name);

    // set syscfg
    syscfg_set(SYSCFG_I_MODEL_SERIES_SET, SYSCFG_N_MODEL_SERIES_SET, final_model_name);

    lv_obj_del(target->obj_front);
    create_custom_msg_box(str, target->obj_back, NULL, LV_EVENT_CLICKED);
  }
}

static void set_label_text(lv_obj_t *label, const char *text, int target_length) {
  char temp[target_length + 1];
  int text_len = strlen(text);
  LOGI(TAG, "text : %s", text);

  if (text_len < target_length) {
    strncpy(temp, text, text_len);
    LOGI(TAG, "text len_1 : %d", strlen(temp));
    for (int i = text_len; i < target_length; i++) {
      temp[i] = ' ';
    }
    temp[target_length] = '\0';
  } else {
    strncpy(temp, text, text_len);
    temp[target_length] = '\0';
  }

  LOGI(TAG, "text len_2 : %d", strlen(temp));
  lv_label_set_text(label, temp);
}

static model_series_data_t *model_list_config(char *model_series_name, indicator_model_t model_id) {
  model_series_data_t *model_data = (model_series_data_t *)malloc(sizeof(model_series_data_t));
  if (model_data == NULL) {
    LOGI(TAG, "error");
    return NULL;
  }

  const char **model_name = NULL;  // 모델 이름 배열에 대한 포인터
  int num_elements = 0;            // 요소의 수

  switch (model_id) {
    case MODEL_NONE: {
      static const char *none_model_name[] = { "none" };
      model_data->model_id = model_id;
      model_name = none_model_name;
      num_elements = sizeof(none_model_name) / sizeof(none_model_name[0]);
      break;
    }
    case MODEL_CAS_SW_11: {
      // 순서있음
      static const char *model_name_array[] = { MODEL_SWII_30KG_200G_N, MODEL_SWII_15KG_100G_N, MODEL_SWII_6KG_40G_N };
      model_data->model_id = model_id;
      model_name = model_name_array;
      num_elements = sizeof(model_name_array) / sizeof(model_name_array[0]);
      break;
    }
    case MODEL_AND_CB_12K: {
      // 순서있음
      static const char *model_name_array[] = { MODEL_CB_310G_0_2G_N, MODEL_CB_3100G_5G_N, MODEL_EK_4100G_5G_N,
                                                MODEL_CB_12KG_50G_N };
      model_data->model_id = model_id;
      model_name = model_name_array;
      num_elements = sizeof(model_name_array) / sizeof(model_name_array[0]);
      break;
    }
    case MODEL_CAS_MW2_H: {
      // 순서있음
      static const char *model_name_array[] = { MODEL_MW2_300G_0_2G_N, MODEL_MW2_3000G_5G_N };
      model_data->model_id = model_id;
      model_name = model_name_array;
      num_elements = sizeof(model_name_array) / sizeof(model_name_array[0]);
      break;
    }
    case MODEL_CAS_EC: {
      // 순서있음
      static const char *model_name_array[] = { MODEL_EC_30KG_100G_N, MODEL_EC_6000G_25G_N };
      model_data->model_id = model_id;
      model_name = model_name_array;
      num_elements = sizeof(model_name_array) / sizeof(model_name_array[0]);
      break;
    }
    case MODEL_CAS_EC_D_SERIES: {
      // 순서있음
      static const char *model_name_array[] = { MODEL_EC_D_30KG_100G_N, MODEL_EC_D_6KG_25G_N };
      model_data->model_id = model_id;
      model_name = model_name_array;
      num_elements = sizeof(model_name_array) / sizeof(model_name_array[0]);
      break;
    }
    case MODEL_CAS_HB_HBI: {
      // 순서있음
      static const char *model_name_array[] = { MODEL_HB_150KG_500G_N, MODEL_HB_75KG_250G_N };
      model_data->model_id = model_id;
      model_name = model_name_array;
      num_elements = sizeof(model_name_array) / sizeof(model_name_array[0]);
      break;
    }
    case MODEL_CAS_WTM500: {
      // 순서있음
      static const char *model_name_array[] = { MODEL_CI_999KG_100G_N, MODEL_CI_99KG_10G_N };
      model_data->model_id = model_id;
      model_name = model_name_array;
      num_elements = sizeof(model_name_array) / sizeof(model_name_array[0]);
      break;
    }

    default: {
      free(model_data);
      return NULL;
    }
  }

  // model_series_name 복사
  model_data->model_series_name = (char *)malloc(strlen(model_series_name) + 1);
  if (model_data->model_series_name == NULL) {
    LOGI(TAG, "error");
    free(model_data);
    return NULL;
  }
  strcpy(model_data->model_series_name, model_series_name);

  // model_name_list 복사
  model_data->model_name_list = (char **)malloc(num_elements * sizeof(char *));
  if (model_data->model_name_list == NULL) {
    LOGI(TAG, "error");
    free(model_data->model_series_name);
    free(model_data);
    return NULL;
  }

  for (int i = 0; i < num_elements; i++) {
    model_data->model_name_list[i] = (char *)malloc(strlen(model_name[i]) + 1);
    if (model_data->model_name_list[i] == NULL) {
      LOGI(TAG, "error");

      // 이미 할당된 메모리 해제
      for (int j = 0; j < i; j++) {
        free(model_data->model_name_list[j]);
      }
      free(model_data->model_name_list);
      free(model_data->model_series_name);
      free(model_data);
      return NULL;
    }
    strcpy(model_data->model_name_list[i], model_name[i]);
  }
  model_data->num_elements = num_elements;

  return model_data;
}

void free_model_series_data(model_series_data_t *model_data) {
  if (model_data != NULL) {
    if (model_data->model_series_name) {
      free(model_data->model_series_name);
      LOGI(TAG, "freeing memory for model_series_name");
    }
    if (model_data->model_name_list) {
      for (int i = 0; i < model_data->num_elements; i++) {
        free(model_data->model_name_list[i]);
        LOGI(TAG, "freeing memory for model_name_list[%d]", i);
      }
      free(model_data->model_name_list);
      LOGI(TAG, "freeing memory for model_name_list");
    }
    free(model_data);
    LOGI(TAG, "freeing memory for model_data");
  }
}
static void model_series_box(lv_obj_t *active_screen, model_series_data_t *model_data) {
  // main panel config
  lv_obj_t *model_series_panel = lv_obj_create(active_screen);
  lv_obj_set_width(model_series_panel, 420);
  lv_obj_set_height(model_series_panel, 300);
  lv_obj_set_x(model_series_panel, 0);
  lv_obj_set_y(model_series_panel, 0);

  lv_obj_set_scroll_snap_x(model_series_panel, LV_SCROLL_SNAP_START);
  lv_obj_set_scroll_dir(model_series_panel, LV_DIR_VER);  // only scroll vertically

  // Use flex flow row to align elements in a row
  lv_obj_set_flex_flow(model_series_panel, LV_FLEX_FLOW_ROW_WRAP);

  lv_obj_set_align(model_series_panel, LV_ALIGN_CENTER);
  // lv_obj_add_flag(Msg_Box_Panel, LV_OBJ_FLAG_HIDDEN);        /// Flags
  lv_obj_clear_flag(model_series_panel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(model_series_panel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(model_series_panel, lv_color_hex(0xdcdcdc), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(model_series_panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  // configuration for top panel in main panel
  // model series 이름과, 취소버튼 위치 때문에 만
  lv_obj_t *model_series_top = lv_obj_create(model_series_panel);
  lv_obj_clear_flag(model_series_top, LV_OBJ_FLAG_SNAPABLE);
  lv_obj_set_width(model_series_top, 400);
  lv_obj_set_height(model_series_top, 60);
  lv_obj_set_x(model_series_top, 0);
  lv_obj_set_y(model_series_top, 0);

  lv_obj_set_align(model_series_top, LV_ALIGN_TOP_MID);
  // lv_obj_add_flag(Msg_Box_Panel, LV_OBJ_FLAG_HIDDEN);        /// Flags
  lv_obj_clear_flag(model_series_top, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(model_series_top, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(model_series_top, lv_color_hex(0xdcdcdc), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(model_series_top, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *label = lv_label_create(model_series_top);
  // lv_obj_clear_flag(label, LV_OBJ_FLAG_SNAPABLE);
  set_label_text(label, model_data->model_series_name, 38);
  // lv_label_set_text(label, "SWII-SERIES                           ");
  lv_obj_set_style_text_font(label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *series_cancel_btn = lv_obj_create(model_series_top);
  // lv_obj_clear_flag(series_btn, LV_OBJ_FLAG_SNAPABLE);
  lv_obj_set_width(series_cancel_btn, 110);
  lv_obj_set_height(series_cancel_btn, 50);
  lv_obj_set_align(series_cancel_btn, LV_ALIGN_RIGHT_MID);
  lv_obj_add_flag(series_cancel_btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(series_cancel_btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(series_cancel_btn, lv_color_hex(0xff0060), LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(series_cancel_btn, model_series_cancel_btn_e_handler, LV_EVENT_CLICKED, model_series_panel);

  lv_obj_t *series_cancel_btn_Label = lv_label_create(series_cancel_btn);
  lv_obj_set_width(series_cancel_btn_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(series_cancel_btn_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(series_cancel_btn_Label, LV_ALIGN_CENTER);
  lv_label_set_text(series_cancel_btn_Label, "취소");
  lv_obj_set_style_text_font(series_cancel_btn_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  // end configuration

  for (int num = 0; num < model_data->num_elements; num++) {
    model_btn[num] = lv_btn_create(model_series_panel);
    lv_obj_set_size(model_btn[num], 120, 90);
    lv_obj_set_style_bg_color(model_btn[num], lv_color_hex(0x232D3F), LV_PART_MAIN | LV_STATE_DEFAULT);

    memset(user_data[num].model_data, 0x00, sizeof(user_data[num].model_data));
    memset(&user_data[num].series_name, 0x00, sizeof(user_data[num].series_name));

    // logic data configuration
    user_data[num].model_id = model_data->model_id;
    user_data[num].model_series_id = get_model_series_enum(model_data->model_name_list[num]);
    user_data[num].obj_front = model_series_panel;
    user_data[num].obj_back = active_screen;
    strcpy(user_data[num].model_data, model_data->model_name_list[num]);
    strcpy(user_data[num].series_name, model_data->model_series_name);
    lv_obj_add_event_cb(model_btn[num], model_series_btn_e_handler, LV_EVENT_CLICKED, &user_data[num]);
    lv_obj_t *label = lv_label_create(model_btn[num]);

    if (num == 3) {
      lv_label_set_text(label, model_data->model_name_list[num]);
      lv_obj_set_style_text_font(label, &NanumBar18, LV_PART_MAIN | LV_STATE_DEFAULT);
      // 특정 객체가 스냅 기능을 통해 자동 정렬되지 않도록 하고 싶을 때
      lv_obj_clear_flag(model_btn[num], LV_OBJ_FLAG_SNAPABLE);

    } else {
      lv_label_set_text(label, model_data->model_name_list[num]);
      lv_obj_set_style_text_font(label, &NanumBar18, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    lv_obj_center(label);
  }
  // 정렬을 위한 함수 실행
  lv_obj_update_snap(model_series_panel, LV_ANIM_OFF);

  free_model_series_data(model_data);
}

static void event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    LV_UNUSED(obj);
    char str_buf[20] = { 0 };
    strncpy(str_buf, lv_list_get_btn_text(indicator_list, obj), sizeof(str_buf));
    LOGI(TAG, "Clicked: %s", str_buf);
    // syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);
    if (strncmp(str_buf, "BX11", 4) == 0) {
      create_custom_msg_box("선택된 모델은 \nBX11 입니다.", ui_Indicator_Model_Select_Screen, NULL, LV_EVENT_CLICKED);
      syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);

    } else if (strncmp(str_buf, "CI-SERIES", 9) == 0) {
      model_series_box(ui_Indicator_Model_Select_Screen, model_list_config("CI", MODEL_CAS_WTM500));
      // create_custom_msg_box("선택된 모델은 \nWTM-500 입니다.", ui_Indicator_Model_Select_Screen, NULL,
      //                       LV_EVENT_CLICKED);
      syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);

    } else if (strncmp(str_buf, "NT-301A", 7) == 0) {
      create_custom_msg_box(":선택된 모델은 \nNT-301A 입니다.", ui_Indicator_Model_Select_Screen, NULL,
                            LV_EVENT_CLICKED);
      syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);

    } else if (strncmp(str_buf, "EC-D", 4) == 0) {
      model_series_box(ui_Indicator_Model_Select_Screen, model_list_config("EC-D", MODEL_CAS_EC_D_SERIES));
      // create_custom_msg_box("선택된 모델은 \nEC-D Series 입니다.", ui_Indicator_Model_Select_Screen, NULL,
      //                       LV_EVENT_CLICKED);
      syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);
    } else if (strncmp(str_buf, "EC", 2) == 0) {
      model_series_box(ui_Indicator_Model_Select_Screen, model_list_config("EC", MODEL_CAS_EC));
      // create_custom_msg_box("선택된 모델은 \nEC Series 입니다.", ui_Indicator_Model_Select_Screen, NULL,
      //                       LV_EVENT_CLICKED);
      syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);
    } else if (strncmp(str_buf, "CB-SERIES", 9) == 0) {
      model_series_box(ui_Indicator_Model_Select_Screen, model_list_config("AND_CB/EK", MODEL_AND_CB_12K));
      syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);

    } else if (strncmp(str_buf, "PW-200", 6) == 0) {
      create_custom_msg_box("선택된 모델은 \nPW-200 입니다.", ui_Indicator_Model_Select_Screen, NULL, LV_EVENT_CLICKED);
      syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);

    } else if (strncmp(str_buf, "SWII-CS", 7) == 0) {
      model_series_box(ui_Indicator_Model_Select_Screen, model_list_config("SWII", MODEL_CAS_SW_11));
      syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);

    } else if (strncmp(str_buf, "INNOTEM-T28", 11) == 0) {
      create_custom_msg_box("선택된 모델은 \nINNOTEM T28 입니다.", ui_Indicator_Model_Select_Screen, NULL,
                            LV_EVENT_CLICKED);
      syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);

    } else if (strncmp(str_buf, "MWII-H", 6) == 0) {
      model_series_box(ui_Indicator_Model_Select_Screen, model_list_config("MWII", MODEL_CAS_MW2_H));
      // create_custom_msg_box("선택된 모델은 \nMWII-H 입니다.", ui_Indicator_Model_Select_Screen, NULL,
      // LV_EVENT_CLICKED);
      syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);

    } else if (strncmp(str_buf, "HB/HBI", 6) == 0) {
      model_series_box(ui_Indicator_Model_Select_Screen, model_list_config("HB/HBI", MODEL_CAS_HB_HBI));
      // create_custom_msg_box("선택된 모델은 \nHB/HBI 입니다.", ui_Indicator_Model_Select_Screen, NULL,
      // LV_EVENT_CLICKED);
      syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);

    } else if (strncmp(str_buf, "DB-1/1H", 7) == 0) {
      create_custom_msg_box("선택된 모델은 \nDB-1/1H 입니다.", ui_Indicator_Model_Select_Screen, NULL,
                            LV_EVENT_CLICKED);
      syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);

    } else if (strncmp(str_buf, "DB-2", 4) == 0) {
      create_custom_msg_box("선택된 모델은 \nDB-2 입니다.", ui_Indicator_Model_Select_Screen, NULL, LV_EVENT_CLICKED);
      syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);

    } else if (strncmp(str_buf, "none", 4) == 0) {
      //   create_custom_msg_box("다시 선택해 주십시오.", ui_Indicator_Model_Select_Screen, NULL, LV_EVENT_CLICKED);
      model_series_box(ui_Indicator_Model_Select_Screen, model_list_config("none", MODEL_NONE));
      syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);
    }
  }
  lv_obj_set_style_bg_color(ui_Indicator_Model_Select_Screen_Comfirm_Btn, lv_palette_main(LV_PALETTE_LIGHT_GREEN),
                            LV_PART_MAIN);
}

static void Speaker_Btn_Yes_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    syscfg_set(SYSCFG_I_SPEAKER, SYSCFG_N_SPEAKER, "ON");
    lv_label_set_text(ui_SPK_Set_State_Label, "음성: ON");
    // 다시 원 상태로 돌아온다면?
    lv_obj_set_style_bg_color(ui_Indicator_Model_Select_Screen_Comfirm_Btn, lv_palette_main(LV_PALETTE_LIGHT_GREEN),
                              LV_PART_MAIN);
    memory_allocation_manger();
  } else {
    memory_allocation_manger();
  }
}
static void Speaker_Btn_No_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    syscfg_set(SYSCFG_I_SPEAKER, SYSCFG_N_SPEAKER, "OFF");
    lv_label_set_text(ui_SPK_Set_State_Label, "음성: OFF");
    // 다시 원 상태로 돌아온다면?
    lv_obj_set_style_bg_color(ui_Indicator_Model_Select_Screen_Comfirm_Btn, lv_palette_main(LV_PALETTE_LIGHT_GREEN),
                              LV_PART_MAIN);
    memory_allocation_manger();
  } else {
    memory_allocation_manger();
  }
}

static void OTA_Mode_Btn_Yes_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    if (strncmp(usb_mode, "OTA", 3) == 0) {
      syscfg_set(SYSCFG_I_USB_MODE, SYSCFG_N_USB_MODE, "MSC");
    } else if (strncmp(usb_mode, "MSC", 3) == 0) {
      syscfg_set(SYSCFG_I_USB_MODE, SYSCFG_N_USB_MODE, "OTA");
    }
    lv_obj_set_style_bg_color(ui_Indicator_Model_Select_Screen_Comfirm_Btn, lv_palette_main(LV_PALETTE_LIGHT_GREEN),
                              LV_PART_MAIN);
    memory_allocation_manger();
  } else {
    memory_allocation_manger();
  }
}

static void ui_Indicator_Model_Speaker_Btn_e_hendler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  char s_buf[100] = { 0 };
  if (code == LV_EVENT_CLICKED) {
    LOGE(TAG, "Speaker Set");
    snprintf(s_buf, sizeof(s_buf), "음성지원을 사용하시겠습니까?");
    create_custom_msg_box_type_1(s_buf, ui_Indicator_Model_Select_Screen, Speaker_Btn_Yes_event_cb,
                                 Speaker_Btn_No_event_cb, LV_EVENT_CLICKED);
  }
}

static void Printer_Btn_Yes_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    syscfg_set(SYSCFG_I_PRINTER, SYSCFG_N_PRINTER, "ON");
    lv_label_set_text(ui_PRT_Set_State_Label, "PRT : ON");
    // 다시 원 상태로 돌아온다면?
    lv_obj_set_style_bg_color(ui_Indicator_Model_Select_Screen_Comfirm_Btn, lv_palette_main(LV_PALETTE_LIGHT_GREEN),
                              LV_PART_MAIN);
    memory_allocation_manger();
  } else {
    memory_allocation_manger();
  }
}

static void Printer_Btn_No_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    syscfg_set(SYSCFG_I_PRINTER, SYSCFG_N_PRINTER, "OFF");
    lv_label_set_text(ui_PRT_Set_State_Label, "PRT : OFF");
    // 다시 원 상태로 돌아온다면?
    lv_obj_set_style_bg_color(ui_Indicator_Model_Select_Screen_Comfirm_Btn, lv_palette_main(LV_PALETTE_LIGHT_GREEN),
                              LV_PART_MAIN);
    memory_allocation_manger();
  } else {
    memory_allocation_manger();
  }
}

static void ui_Indicator_Model_Printer_Btn_e_hendler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  char s_buf[100] = { 0 };
  if (code == LV_EVENT_CLICKED) {
    LOGE(TAG, "Printer Set");
    snprintf(s_buf, sizeof(s_buf), "프린터를 사용하시겠습니까?");
    create_custom_msg_box_type_1(s_buf, ui_Indicator_Model_Select_Screen, Printer_Btn_Yes_event_cb,
                                 Printer_Btn_No_event_cb, LV_EVENT_CLICKED);
  }
}

static void ui_Indicator_Model_OTA_MODE_Btn_e_hendler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  char s_buf[100] = { 0 };
  if (code == LV_EVENT_CLICKED) {
    LOGE(TAG, "OTA_MODE");
    syscfg_get(SYSCFG_I_USB_MODE, SYSCFG_N_USB_MODE, usb_mode, sizeof(usb_mode));
    snprintf(s_buf, sizeof(s_buf), "USB MODE를 변경할까요?\n현재모드:%s", usb_mode);
    LOGE(TAG, "%s", s_buf);
    create_custom_msg_box(s_buf, ui_Indicator_Model_Select_Screen, OTA_Mode_Btn_Yes_event_cb, LV_EVENT_CLICKED);
  }
}

static void ui_Indicator_Model_Select_Screen_back_Btn_e_hendler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    _ui_screen_change(&ui_Main_Screen, LV_SCR_LOAD_ANIM_FADE_ON, 100, 100, &ui_main_screen_init);
    lv_obj_clean(ui_Indicator_Model_Select_Screen);  // Function execution to prevent memory leaks
  }
}

static void mbox_close_reboot_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    esp_restart();
  }
}

static void ui_Indicator_Model_Select_Screen_Comfirm_Btn_e_hendler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    //_ui_screen_change(&ui_Main_Screen, LV_SCR_LOAD_ANIM_FADE_ON, 100, 100, &ui_main_screen_init);
    create_custom_msg_box("재시작 하시겠습니까?", ui_Indicator_Model_Select_Screen, mbox_close_reboot_cb,
                          LV_EVENT_CLICKED);
  }
}

// static void ui_SPK_Set_State_Label_e_hendler(lv_event_t *e) {
//   lv_obj_t *target = lv_event_get_target(e);
//   lv_event_code_t code = lv_event_get_code(e);
//   if (code = LV_EVENT_READY) {
//     lv_label_set_text(target, "음성:ON");
//   }
// }

// static void ui_USB_Set_State_Label_e_hendler(lv_event_t *e) {
//   lv_obj_t *target = lv_event_get_target(e);
//   lv_event_code_t code = lv_event_get_code(e);
//   if (code = LV_EVENT_READY) {
//     lv_label_set_text(target, "USB:MSC");
//   }
// }

void ui_indicator_model_select_screen_init(void) {
  /*Create a list*/
  ui_Indicator_Model_Select_Screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_Indicator_Model_Select_Screen, LV_OBJ_FLAG_SCROLLABLE);  /// Flags

  indicator_list = lv_list_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_size(indicator_list, 200, 250);
  lv_obj_set_x(indicator_list, -10);
  lv_obj_set_y(indicator_list, 0);
  lv_obj_set_align(indicator_list, LV_ALIGN_CENTER);

  /*Add buttons to the list*/
  lv_obj_t *btn;
  lv_list_add_text(indicator_list, "CAS");
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "CI-SERIES");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  // btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "NT-301A");
  // lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "EC-D");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "EC");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "SWII-CS");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "MWII-H");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "HB/HBI");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "DB-1/1H");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  // btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "DB-2");
  // lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

  lv_list_add_text(indicator_list, "OTHER");
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "BX11");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "CB-SERIES");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "PW-200");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "INNOTEM-T28");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "none");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

  lv_list_add_text(indicator_list, "ETC");
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_OK, "none");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_CLOSE, "none");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

  ui_Indicator_Model_Select_Screen_Comfirm_Btn = lv_btn_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_Indicator_Model_Select_Screen_Comfirm_Btn, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_Indicator_Model_Select_Screen_Comfirm_Btn, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_Indicator_Model_Select_Screen_Comfirm_Btn, 160);
  lv_obj_set_y(ui_Indicator_Model_Select_Screen_Comfirm_Btn, 90);
  lv_obj_set_align(ui_Indicator_Model_Select_Screen_Comfirm_Btn, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Indicator_Model_Select_Screen_Comfirm_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_Indicator_Model_Select_Screen_Comfirm_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_Indicator_Model_Select_Screen_Comfirm_Btn, lv_color_hex(0xff0060),
                            LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_Indicator_Model_Select_Screen_Comfirm_Btn,
                      ui_Indicator_Model_Select_Screen_Comfirm_Btn_e_hendler, LV_EVENT_ALL, NULL);

  ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label = lv_label_create(ui_Indicator_Model_Select_Screen_Comfirm_Btn);
  lv_obj_set_width(ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label, -3);
  lv_obj_set_y(ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label, 1);
  lv_label_set_text(ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label, "완 료");
  lv_obj_set_style_text_font(ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label, &NanumBar24,
                             LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_Indicator_Model_Select_Screen_Back_Btn = lv_btn_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_Indicator_Model_Select_Screen_Back_Btn, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_Indicator_Model_Select_Screen_Back_Btn, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_Indicator_Model_Select_Screen_Back_Btn, 160);
  lv_obj_set_y(ui_Indicator_Model_Select_Screen_Back_Btn, 30);
  lv_obj_set_align(ui_Indicator_Model_Select_Screen_Back_Btn, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Indicator_Model_Select_Screen_Back_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_Indicator_Model_Select_Screen_Back_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_Indicator_Model_Select_Screen_Back_Btn, lv_color_hex(0x0E04F8),
                            LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_Indicator_Model_Select_Screen_Back_Btn, ui_Indicator_Model_Select_Screen_back_Btn_e_hendler,
                      LV_EVENT_ALL, NULL);

  lv_obj_t *ui_Indicator_Model_Select_Screen_Back_Btn_Label =
      lv_label_create(ui_Indicator_Model_Select_Screen_Back_Btn);
  lv_obj_set_width(ui_Indicator_Model_Select_Screen_Back_Btn_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Indicator_Model_Select_Screen_Back_Btn_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Indicator_Model_Select_Screen_Back_Btn_Label, -3);
  lv_obj_set_y(ui_Indicator_Model_Select_Screen_Back_Btn_Label, 1);
  lv_label_set_text(ui_Indicator_Model_Select_Screen_Back_Btn_Label, "모 드");
  lv_obj_set_style_text_font(ui_Indicator_Model_Select_Screen_Back_Btn_Label, &NanumBar24,
                             LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_Indicator_Model_OTA_MODE_Btn = lv_btn_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_Indicator_Model_OTA_MODE_Btn, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_Indicator_Model_OTA_MODE_Btn, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_Indicator_Model_OTA_MODE_Btn, 160);
  lv_obj_set_y(ui_Indicator_Model_OTA_MODE_Btn, -30);
  lv_obj_set_align(ui_Indicator_Model_OTA_MODE_Btn, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Indicator_Model_OTA_MODE_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_Indicator_Model_OTA_MODE_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_Indicator_Model_OTA_MODE_Btn, lv_color_hex(0x0E04F8), LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_Indicator_Model_OTA_MODE_Btn, ui_Indicator_Model_OTA_MODE_Btn_e_hendler, LV_EVENT_ALL, NULL);

  lv_obj_t *ui_Indicator_Model_OTA_MODE_Btn_Label = lv_label_create(ui_Indicator_Model_OTA_MODE_Btn);
  lv_obj_set_width(ui_Indicator_Model_OTA_MODE_Btn_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Indicator_Model_OTA_MODE_Btn_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Indicator_Model_OTA_MODE_Btn_Label, -3);
  lv_obj_set_y(ui_Indicator_Model_OTA_MODE_Btn_Label, 1);
  lv_label_set_text(ui_Indicator_Model_OTA_MODE_Btn_Label, "USB");
  lv_obj_set_style_text_font(ui_Indicator_Model_OTA_MODE_Btn_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_Indicator_Model_Speaker_Btn = lv_btn_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_Indicator_Model_Speaker_Btn, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_Indicator_Model_Speaker_Btn, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_Indicator_Model_Speaker_Btn, 160);
  lv_obj_set_y(ui_Indicator_Model_Speaker_Btn, -90);
  lv_obj_set_align(ui_Indicator_Model_Speaker_Btn, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Indicator_Model_Speaker_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_Indicator_Model_Speaker_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_Indicator_Model_Speaker_Btn, lv_color_hex(0x0E04F8), LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_Indicator_Model_Speaker_Btn, ui_Indicator_Model_Speaker_Btn_e_hendler, LV_EVENT_ALL, NULL);

  lv_obj_t *ui_Indicator_Model_Speaker_Btn_Label = lv_label_create(ui_Indicator_Model_Speaker_Btn);
  lv_obj_set_width(ui_Indicator_Model_Speaker_Btn_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Indicator_Model_Speaker_Btn_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Indicator_Model_Speaker_Btn_Label, -3);
  lv_obj_set_y(ui_Indicator_Model_Speaker_Btn_Label, 1);
  lv_label_set_text(ui_Indicator_Model_Speaker_Btn_Label, "SPK");
  lv_obj_set_style_text_font(ui_Indicator_Model_Speaker_Btn_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_Name_Set_State_Label = lv_label_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_Name_Set_State_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Name_Set_State_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Name_Set_State_Label, 10);
  lv_obj_set_y(ui_Name_Set_State_Label, 20);
  lv_obj_set_align(ui_Name_Set_State_Label, LV_ALIGN_TOP_LEFT);
  lv_label_set_text(ui_Name_Set_State_Label, "설정상태");
  lv_obj_set_style_text_font(ui_Name_Set_State_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SPK_Set_State_Label = lv_label_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_SPK_Set_State_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SPK_Set_State_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_SPK_Set_State_Label, 10);
  lv_obj_set_y(ui_SPK_Set_State_Label, 70);
  lv_obj_set_align(ui_SPK_Set_State_Label, LV_ALIGN_TOP_LEFT);
  lv_label_set_text_fmt(ui_SPK_Set_State_Label, "음성 : %s", speaker_set);
  lv_obj_set_style_text_font(ui_SPK_Set_State_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_USB_Set_State_Label = lv_label_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_USB_Set_State_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_USB_Set_State_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_USB_Set_State_Label, 10);
  lv_obj_set_y(ui_USB_Set_State_Label, 120);
  lv_obj_set_align(ui_USB_Set_State_Label, LV_ALIGN_TOP_LEFT);
  lv_label_set_text_fmt(ui_USB_Set_State_Label, "USB : %s", usb_mode);
  lv_obj_set_style_text_font(ui_USB_Set_State_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_PRT_Set_State_Label = lv_label_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_PRT_Set_State_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_PRT_Set_State_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_PRT_Set_State_Label, 10);
  lv_obj_set_y(ui_PRT_Set_State_Label, 170);
  lv_obj_set_align(ui_PRT_Set_State_Label, LV_ALIGN_TOP_LEFT);
  lv_label_set_text_fmt(ui_PRT_Set_State_Label, "PRT : %s", printer_set);
  lv_obj_set_style_text_font(ui_PRT_Set_State_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_Indicator_Model_Printer_Btn = lv_btn_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_Indicator_Model_Printer_Btn, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_Indicator_Model_Printer_Btn, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_Indicator_Model_Printer_Btn, 10);
  lv_obj_set_y(ui_Indicator_Model_Printer_Btn, -20);
  lv_obj_set_align(ui_Indicator_Model_Printer_Btn, LV_ALIGN_BOTTOM_LEFT);
  lv_obj_add_flag(ui_Indicator_Model_Printer_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_Indicator_Model_Printer_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_Indicator_Model_Printer_Btn, lv_color_hex(0x0E04F8), LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_Indicator_Model_Printer_Btn, ui_Indicator_Model_Printer_Btn_e_hendler, LV_EVENT_ALL, NULL);

  lv_obj_t *ui_Indicator_Model_Printer_Btn_Label = lv_label_create(ui_Indicator_Model_Printer_Btn);
  lv_obj_set_width(ui_Indicator_Model_Printer_Btn_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Indicator_Model_Printer_Btn_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Indicator_Model_Printer_Btn_Label, -3);
  lv_obj_set_y(ui_Indicator_Model_Printer_Btn_Label, 1);
  lv_label_set_text(ui_Indicator_Model_Printer_Btn_Label, "PRT");
  lv_obj_set_style_text_font(ui_Indicator_Model_Printer_Btn_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);
}
