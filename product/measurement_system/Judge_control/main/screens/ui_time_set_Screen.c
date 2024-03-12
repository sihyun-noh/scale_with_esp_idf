#include "../ui.h"
#include "time_api.h"
#include "log.h"
#include "time.h"

static const char *TAG = "ui_time_set_screen";
static lv_style_t style_clock;
static struct tm date_time;

#define UI_TEXT_LAEL(name, posx, poy, text)                                                             \
  lv_obj_t *name = lv_label_create(ui_Time_Date_Set_ScreenPanel);                                       \
  lv_obj_set_width(name, LV_SIZE_CONTENT);                                                              \
  lv_obj_set_height(name, LV_SIZE_CONTENT);                                                             \
  lv_obj_set_x(name, posx);                                                                             \
  lv_obj_set_y(name, poy);                                                                              \
  lv_obj_set_align(name, LV_ALIGN_CENTER);                                                              \
  lv_label_set_text(name, text);                                                                        \
  lv_obj_set_style_text_color(name, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT); \
  lv_obj_set_style_text_opa(name, 255, LV_PART_MAIN | LV_STATE_DEFAULT);                                \
  lv_obj_set_style_text_font(name, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

void ui_Time_Date_Set_Screen_Apply_Btn_e_hendler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    set_local_time(date_time.tm_year, date_time.tm_mon, date_time.tm_mday, date_time.tm_hour, date_time.tm_min,
                   date_time.tm_sec);
  }
}

void ui_Time_Date_Set_Screen_Comfirm_Btn_e_hendler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    _ui_screen_change(&ui_Main_Screen, LV_SCR_LOAD_ANIM_FADE_ON, 100, 500, &ui_main_screen_init);
  }
}
static void year_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    char buf[32];
    lv_roller_get_selected_str(obj, buf, sizeof(buf));
    date_time.tm_year = atoi(buf);
    LOGI(TAG, "Selected value: %s", buf);
  }
}

static void month_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    char buf[32];
    lv_roller_get_selected_str(obj, buf, sizeof(buf));
    date_time.tm_mon = atoi(buf);
    LOGI(TAG, "Selected value: %s", buf);
  }
}

static void day_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    char buf[32];
    lv_roller_get_selected_str(obj, buf, sizeof(buf));
    date_time.tm_mday = atoi(buf);
    LOGI(TAG, "Selected value: %s", buf);
  }
}

static void hour_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    char buf[32];
    lv_roller_get_selected_str(obj, buf, sizeof(buf));
    date_time.tm_hour = atoi(buf);
    LOGI(TAG, "Selected value: %s", buf);
  }
}

static void min_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    char buf[32];
    lv_roller_get_selected_str(obj, buf, sizeof(buf));
    date_time.tm_min = atoi(buf);
    LOGI(TAG, "Selected value: %s", buf);
  }
}

static void sec_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    char buf[32];
    lv_roller_get_selected_str(obj, buf, sizeof(buf));
    date_time.tm_sec = atoi(buf);
    LOGI(TAG, "Selected value: %s", buf);
  }
}

void ui_time_set_screen_init(void) {
  ui_Time_Date_Set_Screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_Time_Date_Set_Screen, LV_OBJ_FLAG_SCROLLABLE);  /// Flags

  lv_obj_t *ui_Time_Date_Set_ScreenPanel = lv_obj_create(ui_Time_Date_Set_Screen);
  lv_obj_set_width(ui_Time_Date_Set_ScreenPanel, 420);
  lv_obj_set_height(ui_Time_Date_Set_ScreenPanel, 260);
  lv_obj_set_x(ui_Time_Date_Set_ScreenPanel, 0);
  lv_obj_set_y(ui_Time_Date_Set_ScreenPanel, 1);
  lv_obj_set_align(ui_Time_Date_Set_ScreenPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_Time_Date_Set_ScreenPanel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags

  ui_Time_Date_Set_Screen_Time_Label = lv_label_create(ui_Time_Date_Set_ScreenPanel);
  lv_obj_set_width(ui_Time_Date_Set_Screen_Time_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Time_Date_Set_Screen_Time_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Time_Date_Set_Screen_Time_Label, -20);
  lv_obj_set_y(ui_Time_Date_Set_Screen_Time_Label, 100);
  lv_obj_set_align(ui_Time_Date_Set_Screen_Time_Label, LV_ALIGN_CENTER);
  lv_obj_add_style(ui_Time_Date_Set_Screen_Time_Label, &style_clock, 0);
  lv_label_set_text(ui_Time_Date_Set_Screen_Time_Label, "11:00:00");
  lv_label_set_long_mode(ui_Time_Date_Set_Screen_Time_Label, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_color(ui_Time_Date_Set_Screen_Time_Label, lv_palette_main(LV_PALETTE_GREY),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Time_Date_Set_Screen_Time_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Time_Date_Set_Screen_Time_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Time_Date_Set_Screen_Date_Label = lv_label_create(ui_Time_Date_Set_ScreenPanel);
  lv_obj_set_width(ui_Time_Date_Set_Screen_Date_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Time_Date_Set_Screen_Date_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Time_Date_Set_Screen_Date_Label, -130);
  lv_obj_set_y(ui_Time_Date_Set_Screen_Date_Label, 100);
  lv_obj_set_align(ui_Time_Date_Set_Screen_Date_Label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_Time_Date_Set_Screen_Date_Label, "2024-03-06");
  lv_obj_set_style_text_color(ui_Time_Date_Set_Screen_Date_Label, lv_palette_main(LV_PALETTE_GREY),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Time_Date_Set_Screen_Date_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Time_Date_Set_Screen_Date_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  /*A style to make the selected option larger*/
  static lv_style_t style_sel;
  lv_style_init(&style_sel);
  lv_style_set_text_font(&style_sel, &NanumBar24);

  const char *opts_year =
      "2021\n2022\n2023\n2024\n2025\n2026\n2027\n2028\n2029\n2030\n2031\n2032\n2033\n2034\n2035\n2036\n2037\n2038\n2039"
      "\n2040";
  const char *opts_month = "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12";
  const char *opts_day =
      "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n3"
      "1";
  const char *opts_hour = "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23";
  const char *opts_min =
      "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30"
      "\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58"
      "\n59";
  const char *opts_sec =
      "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30"
      "\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58"
      "\n59";

  UI_TEXT_LAEL(title_text_label, -140, -105, "날짜&시간 설정")
  UI_TEXT_LAEL(year_text_label, -175, -70, "년")
  UI_TEXT_LAEL(month_text_label, -105, -70, "월")
  UI_TEXT_LAEL(day_text_label, -35, -70, "일")
  UI_TEXT_LAEL(hour_text_label, 35, -70, "시간")
  UI_TEXT_LAEL(min_text_label, 105, -70, "분")
  UI_TEXT_LAEL(sec_text_label, 175, -70, "초")

  /*A roller on the middle with center aligned text, and auto (default) width*/
  lv_obj_t *roller_year = lv_roller_create(ui_Time_Date_Set_ScreenPanel);
  lv_roller_set_options(roller_year, opts_year, LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(roller_year, 3);
  lv_obj_add_style(roller_year, &style_sel, LV_PART_SELECTED);
  lv_obj_set_width(roller_year, 60);
  lv_obj_align(roller_year, LV_ALIGN_CENTER, -175, 0);
  lv_obj_add_event_cb(roller_year, year_event_handler, LV_EVENT_ALL, NULL);
  lv_roller_set_selected(roller_year, 5, LV_ANIM_OFF);

  /*A roller on the middle with center aligned text, and auto (default) width*/
  lv_obj_t *roller_month = lv_roller_create(ui_Time_Date_Set_ScreenPanel);
  lv_roller_set_options(roller_month, opts_month, LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(roller_month, 3);
  lv_obj_add_style(roller_month, &style_sel, LV_PART_SELECTED);
  lv_obj_set_width(roller_month, 60);
  lv_obj_align(roller_month, LV_ALIGN_CENTER, -105, 0);
  lv_obj_add_event_cb(roller_month, month_event_handler, LV_EVENT_ALL, NULL);
  lv_roller_set_selected(roller_month, 5, LV_ANIM_OFF);

  /*A roller on the middle with center aligned text, and auto (default) width*/
  lv_obj_t *roller_day = lv_roller_create(ui_Time_Date_Set_ScreenPanel);
  lv_roller_set_options(roller_day, opts_day, LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(roller_day, 3);
  lv_obj_add_style(roller_day, &style_sel, LV_PART_SELECTED);
  lv_obj_set_width(roller_day, 60);
  lv_obj_align(roller_day, LV_ALIGN_CENTER, -35, 0);
  lv_obj_add_event_cb(roller_day, day_event_handler, LV_EVENT_ALL, NULL);
  lv_roller_set_selected(roller_day, 5, LV_ANIM_OFF);

  /*A roller on the middle with center aligned text, and auto (default) width*/
  lv_obj_t *roller_hour = lv_roller_create(ui_Time_Date_Set_ScreenPanel);
  lv_roller_set_options(roller_hour, opts_hour, LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(roller_hour, 3);
  lv_obj_add_style(roller_hour, &style_sel, LV_PART_SELECTED);
  lv_obj_set_width(roller_hour, 60);
  lv_obj_align(roller_hour, LV_ALIGN_CENTER, 35, 0);
  lv_obj_add_event_cb(roller_hour, hour_event_handler, LV_EVENT_ALL, NULL);
  lv_roller_set_selected(roller_hour, 5, LV_ANIM_OFF);

  /*A roller on the middle with center aligned text, and auto (default) width*/
  lv_obj_t *roller_min = lv_roller_create(ui_Time_Date_Set_ScreenPanel);
  lv_roller_set_options(roller_min, opts_min, LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(roller_min, 3);
  lv_obj_add_style(roller_min, &style_sel, LV_PART_SELECTED);
  lv_obj_set_width(roller_min, 60);
  lv_obj_align(roller_min, LV_ALIGN_CENTER, 105, 0);
  lv_obj_add_event_cb(roller_min, min_event_handler, LV_EVENT_ALL, NULL);
  lv_roller_set_selected(roller_min, 5, LV_ANIM_OFF);

  /*A roller on the middle with center aligned text, and auto (default) width*/
  lv_obj_t *roller_sec = lv_roller_create(ui_Time_Date_Set_ScreenPanel);
  lv_roller_set_options(roller_sec, opts_sec, LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(roller_sec, 3);
  lv_obj_add_style(roller_sec, &style_sel, LV_PART_SELECTED);
  lv_obj_set_width(roller_sec, 60);
  lv_obj_align(roller_sec, LV_ALIGN_CENTER, 175, 0);
  lv_obj_add_event_cb(roller_sec, sec_event_handler, LV_EVENT_ALL, NULL);
  lv_roller_set_selected(roller_sec, 5, LV_ANIM_OFF);

  lv_obj_t *ui_Time_Date_Set_Screen_Apply_Btn = lv_btn_create(ui_Time_Date_Set_ScreenPanel);
  lv_obj_set_width(ui_Time_Date_Set_Screen_Apply_Btn, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_Time_Date_Set_Screen_Apply_Btn, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_Time_Date_Set_Screen_Apply_Btn, 80);
  lv_obj_set_y(ui_Time_Date_Set_Screen_Apply_Btn, 90);
  lv_obj_set_align(ui_Time_Date_Set_Screen_Apply_Btn, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Time_Date_Set_Screen_Apply_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_Time_Date_Set_Screen_Apply_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_Time_Date_Set_Screen_Apply_Btn, lv_color_hex(0x0079ff), LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_Time_Date_Set_Screen_Apply_Btn, ui_Time_Date_Set_Screen_Apply_Btn_e_hendler, LV_EVENT_ALL,
                      NULL);

  lv_obj_t *ui_Time_Date_Set_Screen_Apply_Btn_Label = lv_label_create(ui_Time_Date_Set_Screen_Apply_Btn);
  lv_obj_set_width(ui_Time_Date_Set_Screen_Apply_Btn_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Time_Date_Set_Screen_Apply_Btn_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Time_Date_Set_Screen_Apply_Btn_Label, -3);
  lv_obj_set_y(ui_Time_Date_Set_Screen_Apply_Btn_Label, 1);
  lv_label_set_text(ui_Time_Date_Set_Screen_Apply_Btn_Label, "적 용");
  lv_obj_set_style_text_font(ui_Time_Date_Set_Screen_Apply_Btn_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_Time_Date_Set_Screen_Comfirm_Btn = lv_btn_create(ui_Time_Date_Set_ScreenPanel);
  lv_obj_set_width(ui_Time_Date_Set_Screen_Comfirm_Btn, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_Time_Date_Set_Screen_Comfirm_Btn, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_Time_Date_Set_Screen_Comfirm_Btn, 160);
  lv_obj_set_y(ui_Time_Date_Set_Screen_Comfirm_Btn, 90);
  lv_obj_set_align(ui_Time_Date_Set_Screen_Comfirm_Btn, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Time_Date_Set_Screen_Comfirm_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_Time_Date_Set_Screen_Comfirm_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_Time_Date_Set_Screen_Comfirm_Btn, lv_color_hex(0xff0060),
                            LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_Time_Date_Set_Screen_Comfirm_Btn, ui_Time_Date_Set_Screen_Comfirm_Btn_e_hendler, LV_EVENT_ALL,
                      NULL);

  lv_obj_t *ui_Time_Date_Set_Screen_Comfirm_Btn_Label = lv_label_create(ui_Time_Date_Set_Screen_Comfirm_Btn);
  lv_obj_set_width(ui_Time_Date_Set_Screen_Comfirm_Btn_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Time_Date_Set_Screen_Comfirm_Btn_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Time_Date_Set_Screen_Comfirm_Btn_Label, -3);
  lv_obj_set_y(ui_Time_Date_Set_Screen_Comfirm_Btn_Label, 1);
  lv_label_set_text(ui_Time_Date_Set_Screen_Comfirm_Btn_Label, "완 료");
  lv_obj_set_style_text_font(ui_Time_Date_Set_Screen_Comfirm_Btn_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);
}
