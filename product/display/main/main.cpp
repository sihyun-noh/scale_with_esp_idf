/*
    Simple ESP-IDF Demo with WT32-SC01 + LovyanGFX + LVGL8.x
*/
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string>
#include "sdkconfig.h"

static const char *TAG = "MAIN";
#define LV_TICK_PERIOD_MS 1
// #define LGFX_WT32_SC01 // Wireless Tag / Seeed WT32-SC01
// #define WT32_SC01_PLUS // Wireless Tag / Seeed WT32-SC01
#define LGFX_USE_V1  // LovyanGFX version

// #define LGFX_AUTODETECT
#include <LovyanGFX.h>

// #include <LGFX_AUTODETECT.hpp>
#include "conf_WT32SCO1-Plus.h"

#include <lvgl.h>
#include "ui.h"

static LGFX lcd;
/*** Setup screen resolution for LVGL ***/
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

/*** Function declaration ***/
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
void lv_button_demo(void);
static void lv_tick_task(void *arg);

char txt[100];
lv_obj_t *tlabel;  // touch x,y label

extern "C" {

// void ui_reset() {
// lv_slider_set_value(ui_speedSlider, 0, LV_ANIM_OFF);
// lv_obj_clear_flag(ui_speedSlider, LV_OBJ_FLAG_CLICKABLE);
// lv_label_set_text(ui_speedLabel, "0");
// lv_label_set_text(ui_SpeedLabel2, "0");
// lv_label_set_text(ui_SatellitesLabel, "0");
// lv_label_set_text(ui_CoordinatesLabel, "0\n0");
// lv_label_set_text(ui_AltitudeLabel, "0.00");
// }
void app_main(void) {
  lcd.init();  // Initialize LovyanGFX
  // lcd.begin(); // Initialize LovyanGFX
  // lcd.setRotation(2);
  // lcd.setBrightness(255);
  // lcd.fillScreen(TFT_BLACK);
  lv_init();  // Initialize lvgl

  // Setting display to landscape
  // if (lcd.width() < lcd.height())
  lcd.setRotation(lcd.getRotation() ^ 1);

  /* LVGL : Setting up buffer to use for display */
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  /*** LVGL : Setup & Initialize the display device driver ***/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = display_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*** LVGL : Setup & Initialize the input device driver ***/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read;
  lv_indev_drv_register(&indev_drv);

  /* Create and start a periodic timer interrupt to call lv_tick_inc */
  const esp_timer_create_args_t periodic_timer_args = { .callback = &lv_tick_task, .name = "periodic_gui" };
  esp_timer_handle_t periodic_timer;
  ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

  ui_init();
  // ui_reset();
  /*** Create simple label and show LVGL version ***/
  // sprintf(txt, "WT32-SC01 with LVGL v%d.%d.%d", lv_version_major(), lv_version_minor(), lv_version_patch());

  // lv_obj_t *label = lv_label_create(lv_scr_act()); // full screen as the parent
  // lv_label_set_text(label, txt);                   // set label text
  // lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);    // Center but 20 from the top

  // tlabel = lv_label_create(lv_scr_act());         // full screen as the parent
  // lv_label_set_text(tlabel, "Touch:(000,000)");   // set label text
  // lv_obj_align(tlabel, LV_ALIGN_TOP_RIGHT, 0, 0); // Center but 20 from the top

  // lv_button_demo(); // lvl buttons
  unsigned int count = 0;
  unsigned int value = 0;
  while (1) {
    lv_timer_handler(); /* let the GUI do its work */
    // count++;
    // if(count == 10)
    // {
    //  value++;
    //  count = 0;
    // }
    // switch (value)
    // {
    // case 0/* constant-expression */:
    //      lv_label_set_text(ui_speedLabel, "10");
    //     /* code */
    //     break;
    // case 1/* constant-expression */:
    //      lv_label_set_text(ui_speedLabel, "20");
    //     /* code */
    //     break;
    // case 2/* constant-expression */:
    //      lv_label_set_text(ui_speedLabel, "30");
    //     /* code */
    //     break;
    // case 3/* constant-expression */:
    //      lv_label_set_text(ui_speedLabel, "40");
    //     /* code */
    //     break;
    // case 4/* constant-expression */:
    //      lv_label_set_text(ui_speedLabel, "50");
    //     /* code */
    //     break;
    // case 5/* constant-expression */:
    //      value = 0;
    //     /* code */
    //     break;
    // default:
    //     break;
    // }

    vTaskDelay(1);
  }
}
}

/*** Display callback to flush the buffer to screen ***/
void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, w, h);
  lcd.pushColors((uint16_t *)&color_p->full, w * h, true);
  lcd.endWrite();

  lv_disp_flush_ready(disp);
}

/*** Touchpad callback to read the touchpad ***/
void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  bool touched = lcd.getTouch(&touchX, &touchY);

  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;

    // sprintf(txt, "Touch:(%03d,%03d)", touchX, touchY);
    // lv_label_set_text(tlabel, txt); // set label text
  }
}

// /* Counter button event handler */
// static void counter_event_handler(lv_event_t *e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     lv_obj_t *btn = lv_event_get_target(e);
//     if (code == LV_EVENT_CLICKED)
//     {
//         static uint8_t cnt = 0;
//         cnt++;

//         /*Get the first child of the button which is the label and change its text*/
//         lv_obj_t *label = lv_obj_get_child(btn, 0);
//         lv_label_set_text_fmt(label, "Button: %d", cnt);
//         LV_LOG_USER("Clicked");
//         ESP_LOGI(TAG, "Clicked");
//     }
// }

// /* Toggle button event handler */
// static void toggle_event_handler(lv_event_t *e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     if (code == LV_EVENT_VALUE_CHANGED)
//     {
//         LV_LOG_USER("Toggled");
//         ESP_LOGI(TAG, "Toggled");
//     }
// }

// /* Button with counter & Toggle Button */
// void lv_button_demo(void)
// {
//     lv_obj_t *label;

//     // Button with counter
//     lv_obj_t *btn1 = lv_btn_create(lv_scr_act());
//     lv_obj_add_event_cb(btn1, counter_event_handler, LV_EVENT_ALL, NULL);

//     lv_obj_set_pos(btn1, 100, 100); /*Set its position*/
//     lv_obj_set_size(btn1, 120, 50); /*Set its size*/

//     label = lv_label_create(btn1);
//     lv_label_set_text(label, "Button");
//     lv_obj_center(label);

//     // Toggle button
//     lv_obj_t *btn2 = lv_btn_create(lv_scr_act());
//     lv_obj_add_event_cb(btn2, toggle_event_handler, LV_EVENT_ALL, NULL);
//     lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
//     lv_obj_set_pos(btn2, 250, 100); /*Set its position*/
//     lv_obj_set_size(btn2, 120, 50); /*Set its size*/

//     label = lv_label_create(btn2);
//     lv_label_set_text(label, "Toggle Button");
//     lv_obj_center(label);
// }

/* Setting up tick task for lvgl */
static void lv_tick_task(void *arg) {
  (void)arg;
  lv_tick_inc(LV_TICK_PERIOD_MS);
}
