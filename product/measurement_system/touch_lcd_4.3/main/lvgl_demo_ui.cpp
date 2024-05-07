/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

// This demo UI is adapted from LVGL official example: https://docs.lvgl.io/master/examples.html#scatter-chart

#include "lvgl.h"

// static void draw_event_cb(lv_event_t *e) {
//   lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
//   if (dsc->part == LV_PART_ITEMS) {
//     lv_obj_t *obj = lv_event_get_target(e);
//     lv_chart_series_t *ser = lv_chart_get_series_next(obj, NULL);
//     uint32_t cnt = lv_chart_get_point_count(obj);
//     /*Make older value more transparent*/
//     dsc->rect_dsc->bg_opa = (LV_OPA_COVER * dsc->id) / (cnt - 1);

//     /*Make smaller values blue, higher values red*/
//     lv_coord_t *x_array = lv_chart_get_x_array(obj, ser);
//     lv_coord_t *y_array = lv_chart_get_y_array(obj, ser);
//     /*dsc->id is the tells drawing order, but we need the ID of the point being drawn.*/
//     uint32_t start_point = lv_chart_get_x_start_point(obj, ser);
//     uint32_t p_act = (start_point + dsc->id) % cnt; /*Consider start point to get the index of the array*/
//     lv_opa_t x_opa = (x_array[p_act] * LV_OPA_50) / 200;
//     lv_opa_t y_opa = (y_array[p_act] * LV_OPA_50) / 1000;

//     dsc->rect_dsc->bg_color =
//         lv_color_mix(lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_BLUE), x_opa + y_opa);
//   }
// }

// static void add_data(lv_timer_t *timer) {
//   lv_obj_t *chart = timer->user_data;
//   lv_chart_set_next_value2(chart, lv_chart_get_series_next(chart, NULL), lv_rand(0, 200), lv_rand(0, 1000));
// }

// void example_lvgl_demo_ui(lv_disp_t *disp) {
//   lv_obj_t *scr = lv_disp_get_scr_act(disp);
//   lv_obj_t *chart = lv_chart_create(scr);
//   lv_obj_set_size(chart, 200, 150);
//   lv_obj_align(chart, LV_ALIGN_CENTER, 0, 0);
//   lv_obj_add_event_cb(chart, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
//   lv_obj_set_style_line_width(chart, 0, LV_PART_ITEMS); /*Remove the lines*/

//   lv_chart_set_type(chart, LV_CHART_TYPE_SCATTER);

//   lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 5, 5, 5, 1, true, 30);
//   lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 6, 5, true, 50);

//   lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_X, 0, 200);
//   lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 1000);

//   lv_chart_set_point_count(chart, 50);

//   lv_chart_series_t *ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
//   for (int i = 0; i < 50; i++) {
//     lv_chart_set_next_value2(chart, ser, lv_rand(0, 200), lv_rand(0, 1000));
//   }

//   lv_timer_create(add_data, 100, chart);
// }

// static void anim_x_cb(void *var, int32_t v) {
//   lv_obj_set_x(var, v);
// }

// static void anim_size_cb(void *var, int32_t v) {
//   lv_obj_set_size(var, v, v);
// }

// /**
//  * Create a playback animation
//  */
// void lv_example_anim_2(lv_disp_t *disp) {
//   lv_obj_t *scr = lv_disp_get_scr_act(disp);
//   lv_obj_t *obj = lv_obj_create(scr);
//   lv_obj_set_style_bg_color(obj, lv_palette_main(LV_PALETTE_RED), 0);
//   lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);

//   lv_obj_align(obj, LV_ALIGN_LEFT_MID, 10, 0);

//   lv_anim_t a;
//   lv_anim_init(&a);
//   lv_anim_set_var(&a, obj);
//   lv_anim_set_values(&a, 10, 200);
//   lv_anim_set_time(&a, 1000);
//   lv_anim_set_playback_delay(&a, 100);
//   lv_anim_set_playback_time(&a, 300);
//   lv_anim_set_repeat_delay(&a, 500);
//   lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
//   lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);

//   lv_anim_set_exec_cb(&a, anim_size_cb);
//   lv_anim_start(&a);
//   lv_anim_set_exec_cb(&a, anim_x_cb);
//   lv_anim_set_values(&a, 10, 400);
//   lv_anim_start(&a);
// }
