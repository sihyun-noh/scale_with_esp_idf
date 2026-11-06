/**
 * @file eez_ui_binding.cpp
 * @brief 7B 농진원 앱 화면 카드와 제품 서비스의 연결 지도.
 *
 * 찾을 때: Dashboard=render_dashboard(), 모터 카드=detail_*(),
 * 설정 카드=render_setting_views()/setting_*(), 두둑 카드=ridge_*(),
 * 처방 카드=stage_fertilizer_from_ui(), 배출량표=render_calibration_selection().
 * 화면 입력의 RAM 반영은 stage_*(), 영구 저장 요청은 *_save_event()에서 시작한다.
 * 이 파일에서 CAN 송신이나 NVS 쓰기를 직접 실행하지 않는다.
 */
#include "eez_ui_binding.h"

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "dajilog_system_core.h"
#include "dajilog_calibration_catalog.h"
#include "dajilog_ridge_preview.h"
#include "dajilog_flow_calculator.h"
#include "dajilog_ui_adaptor.h"
#include "dajilog_ui_msg.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "generated/waveshare_7b_nongjinwon/src/ui/eez-flow.h"
#include "generated/waveshare_7b_nongjinwon/src/ui/screens.h"
#include "generated/waveshare_7b_nongjinwon/src/ui/vars.h"

namespace {

constexpr uint32_t STRING_ID_RIDGE_COUNT = 0x07010001U;
constexpr uint32_t STRING_ID_RIDGE_WIDTH = 0x07010002U;
constexpr uint32_t STRING_ID_ACTUAL_LENGTH = 0x07010003U;
constexpr uint32_t STRING_ID_NITROGEN_RATE = 0x07010004U;
constexpr uint32_t STRING_ID_DASHBOARD_BASE = 0x07011000U;
constexpr uint32_t STRING_ID_MOTOR_VOLTAGE_BASE = 0x07012000U;
constexpr uint32_t STRING_ID_SETTING_BASE = 0x07013000U;
constexpr size_t MAX_MESSAGES_PER_TICK = 4U;

const char *TAG = "EezUiBinding7B";
dajilog_ui_adaptor_t s_adaptor = {};
bool s_attached = false;
lv_obj_t *s_previous_setting_screen = nullptr;
char s_ridge_count_text[32] = {};
char s_ridge_width_text[32] = {};
char s_actual_length_text[32] = {};
char s_nitrogen_rate_text[32] = {};
char s_operation_text[32] = {};
char s_can_text[32] = {};
char s_vehicle_speed_text[32] = {};
char s_vehicle_target_text[32] = {};
char s_vehicle_signal_text[32] = {};
char s_work_mode_text[32] = {};
char s_recipe_name_text[DAJILOG_RECIPE_NAME_CAPACITY] = {};
char s_alarm_text[DAJILOG_ALARM_TEXT_CAPACITY] = {};
char s_auto_stop_reason_text[DAJILOG_ALARM_TEXT_CAPACITY] = {};
char s_auto_stop_detail_text[96] = {};
char s_auto_stop_zero_text[96] = {};
uint32_t s_auto_stop_acknowledged_sequence = 0;
uint32_t s_auto_stop_presented_sequence = 0;
char s_motor_state_text[DAJILOG_MOTOR_CHANNEL_COUNT][24] = {};
char s_motor_speed_text[DAJILOG_MOTOR_CHANNEL_COUNT][24] = {};
char s_motor_target_text[DAJILOG_MOTOR_CHANNEL_COUNT][24] = {};
char s_motor_load_text[DAJILOG_MOTOR_CHANNEL_COUNT][24] = {};
char s_setting_name_text[2][DAJILOG_RECIPE_NAME_CAPACITY] = {};
char s_driver_summary_text[4][96] = {};
char s_driver_output_state_text[2][24] = {};
char s_detail_selected_driver_text[48] = {};
char s_detail_selected_channel_text[16] = {};
char s_detail_step_text[16] = {};
char s_detail_final_voltage_text[24] = {};
char s_detail_target_summary_text[64] = {};
char s_detail_voltage_summary_text[64] = {};
uint8_t s_detail_motor_index = 0U;
uint8_t s_detail_output_step = 0U;
int8_t s_detail_trim_decivolt = 0;
bool s_detail_service_test = false;
dajilog_driver_setting_t s_driver_draft = {};
bool s_test_held = false;
int64_t s_test_refresh_us = 0;
uint32_t s_sequence = 1000;
uint32_t s_apply_sequence = 0;
int64_t s_apply_deadline = 0;
int64_t s_run_deadline = 0;
uint32_t s_run_request_sequence = 0U;
char s_draft_mode_text[24] = {};
char s_countdown_text[16] = {};
char s_setting_text[FLOW_GLOBAL_VARIABLE_RUN_READINESS_FAULT_REASON_TEXT + 1U][96] = {};
char s_calibration_list_text[2048] = {};
dajilog_product_catalog_t s_product_catalog = {};
int s_last_product_index[DAJILOG_SETTING_ZONE_COUNT] = {-1, -1};
bool s_ridge_screen_was_active = false;
bool s_ridge_form_was_visible = false;
char s_ridge_preview_inputs[256] = {};
uint32_t s_ridge_request_sequence = 0U;
uint32_t s_ridge_pending_topic = 0U;
int64_t s_ridge_request_deadline = 0;
char s_calibration_channel = '\0';
dajilog_calibration_summary_t s_calibration_candidates[DAJILOG_CALIBRATION_MAX_PROFILES] = {};
size_t s_calibration_candidate_count = 0U;
size_t s_calibration_candidate_index = 0U;
size_t s_calibration_row_profile[DAJILOG_CALIBRATION_MAX_PROFILES + DAJILOG_CALIBRATION_MAX_POINTS] = {};
size_t s_calibration_row_count = 0U;
bool s_calibration_screen_was_active = false;
char s_calibration_result[96] = {};
dajilog_setting_zone_id_t s_selected_zone = DAJILOG_SETTING_ZONE_SURFACE;
uint32_t s_setting_request_sequence = 0U;
uint32_t s_setting_request_topic = 0U;
uint32_t s_setting_pending_topic = 0U;
int64_t s_setting_request_deadline = 0;
char s_setting_status[96] = {};
void end_test();
void select_driver(size_t driver);
void detail_close_event(lv_event_t *event);

esp_err_t render_current_recipe();
void render_detail_voltage();
void render_setting_views(bool include_detail = true);
void render_calibration_selection(void);
void render_ridge_profiles(bool load_fields);
void render_ridge_preview();
bool render_run_readiness();
void set_string(uint32_t variable, char *storage, size_t storage_size,
                uint32_t string_id, const char *value);
const char *get_text(uint32_t variable);

void setting_text(uint32_t variable, const char *value)
{
    if (variable < sizeof(s_setting_text) / sizeof(s_setting_text[0])) {
        const char *next = value != nullptr ? value : "";
        if (std::strcmp(s_setting_text[variable], next) == 0 &&
            std::strcmp(get_text(variable), next) == 0) {
            return;
        }
        set_string(variable, s_setting_text[variable],
                   sizeof(s_setting_text[variable]),
                   STRING_ID_SETTING_BASE + variable, value);
    }
}

/** @brief [D-003 S1/S2 카드] 계획 준비량과 Core의 표 기준 추정 시비량을 표시한다. */
void render_application_estimate(const dajilog_dashboard_snapshot_t &snapshot)
{
    dajilog_product_settings_snapshot_t draft = {};
    dajilog_product_settings_snapshot_t applied = {};
    dajilog_ridge_profiles_snapshot_t ridges = {};
    const bool has_draft = dajilog_system_core_get_settings_draft(&draft) == ESP_OK;
    const bool has_applied = dajilog_system_core_get_settings_applied(&applied) == ESP_OK;
    const bool has_ridges = dajilog_system_core_get_ridge_profiles(&ridges) == ESP_OK;
    double width = 0.0, length = 0.0;
    if (has_ridges) {
        for (uint32_t i = 0; i < ridges.count; ++i) {
            if (ridges.profiles[i].id != ridges.selected_id) continue;
            const auto &input = ridges.profiles[i].input;
            width = input.width_m +
                (input.work_width_mode == DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW
                     ? input.furrow_width_m : 0.0);
            length = input.split_section
                ? input.north_length_m + input.south_length_m : input.whole_length_m;
            break;
        }
    }
    const uint32_t preparation_vars[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_1_PREPARATION_KG_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_2_PREPARATION_KG_TEXT,
    };
    const uint32_t accum_vars[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_1_ESTIMATED_ACCUM_KG_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_2_ESTIMATED_ACCUM_KG_TEXT,
    };
    const uint32_t progress_vars[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_1_PROGRESS_PCT_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_2_PROGRESS_PCT_TEXT,
    };
    const uint32_t state_vars[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_1_ESTIMATE_STATE_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_2_ESTIMATE_STATE_TEXT,
    };
    for (size_t zone = 0; zone < 2U; ++zone) {
        const auto &settings = has_applied && applied.zones[zone].configured
                                   ? applied.zones[zone] : draft.zones[zone];
        const bool plan_valid = (has_applied || has_draft) && settings.configured &&
            std::isfinite(width) && std::isfinite(length) &&
            width > 0.0 && length > 0.0 &&
            std::isfinite(settings.rate_kg_per_10a) && settings.rate_kg_per_10a > 0.0f;
        const double preparation = plan_valid
            ? settings.rate_kg_per_10a * width * length / 1000.0 : 0.0;
        char value[32];
        if (plan_valid) std::snprintf(value, sizeof(value), "%.2f", preparation);
        setting_text(preparation_vars[zone], plan_valid ? value : "--");
        if (snapshot.estimate_has_value[zone]) {
            std::snprintf(value, sizeof(value), "%.3f",
                          static_cast<double>(snapshot.estimated_accum_kg[zone]));
            setting_text(accum_vars[zone], value);
        } else setting_text(accum_vars[zone], "--");
        if (plan_valid && snapshot.estimate_has_value[zone]) {
            std::snprintf(value, sizeof(value), "%.1f",
                100.0 * snapshot.estimated_accum_kg[zone] / preparation);
            setting_text(progress_vars[zone], value);
        } else setting_text(progress_vars[zone], "--");
        const char *state = snapshot.operation_state == DAJILOG_OPERATION_RUN_AUTO
            ? snapshot.estimate_valid[zone] ? "표 기준 추정" : "산출 불가"
            : snapshot.estimate_has_value[zone] ? "정지" : "대기";
        setting_text(state_vars[zone], state);
        if (zone != static_cast<size_t>(s_selected_zone)) continue;
        if (snapshot.operation_state == DAJILOG_OPERATION_RUN_AUTO &&
            snapshot.drivers[zone].mode == 1) {
            std::snprintf(value, sizeof(value), "%.3f kg/min",
                static_cast<double>(snapshot.motors[zone * 2U].target_kg_min +
                                    snapshot.motors[zone * 2U + 1U].target_kg_min));
            setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_TOTAL_TARGET_TEXT, value);
        }
        setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_ESTIMATED_ACCUM_KG_TEXT,
                     snapshot.estimate_has_value[zone] ?
                         s_setting_text[accum_vars[zone]] : "--");
        setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_PROGRESS_PCT_TEXT,
                     plan_valid && snapshot.estimate_has_value[zone] ?
                         s_setting_text[progress_vars[zone]] : "--");
        if (snapshot.operation_state == DAJILOG_OPERATION_RUN_AUTO &&
            snapshot.estimate_valid[zone]) {
            std::snprintf(value, sizeof(value), "%.3f",
                          static_cast<double>(snapshot.estimated_kg_min[zone]));
            setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_ESTIMATED_RATE_KG_MIN_TEXT, value);
        } else setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_ESTIMATED_RATE_KG_MIN_TEXT, "--");
        setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_ESTIMATE_STATE_TEXT, state);
    }
}

#ifdef CONFIG_DAJILOG_SETTINGS_DEBUG_LOG
#define SETTINGS_LOGI(format, ...) ESP_LOGI(TAG, format, ##__VA_ARGS__)
#else
#define SETTINGS_LOGI(format, ...) do { } while (0)
#endif

const char *get_text(uint32_t variable)
{
    const eez::Value value = eez::flow::getGlobalVariable(variable);
    const char *text = value.getString();
    return text != nullptr ? text : "";
}

void set_text(uint32_t variable, char *storage, size_t storage_size,
              uint32_t string_id, const char *format, double value)
{
    std::snprintf(storage, storage_size, format, value);
    eez::flow::setGlobalVariable(
        variable, eez::Value::makeStringRef(storage, -1, string_id));
}

void set_string(uint32_t variable, char *storage, size_t storage_size,
                uint32_t string_id, const char *value)
{
    if (value != storage) {
        std::snprintf(storage, storage_size, "%s",
                      value != nullptr ? value : "");
    }
    eez::flow::setGlobalVariable(
        variable, eez::Value::makeStringRef(storage, -1, string_id));
}

const char *operation_text(dajilog_operation_state_t state)
{
    switch (state) {
    case DAJILOG_OPERATION_INITIALIZING: return "초기화";
    case DAJILOG_OPERATION_READY: return "준비";
    case DAJILOG_OPERATION_RUN_AUTO: return "자동 운전";
    case DAJILOG_OPERATION_RUN_MANUAL: return "수동 운전";
    case DAJILOG_OPERATION_STOPPING: return "정지 중";
    case DAJILOG_OPERATION_FAULT: return "오류";
    default: return "부팅";
    }
}

const char *work_mode_text(dajilog_work_mode_t mode)
{
    switch (mode) {
    case DAJILOG_WORK_MODE_MANUAL: return "수동 살포";
    case DAJILOG_WORK_MODE_TEST: return "시험";
    default: return "자동 살포";
    }
}

const char *motor_state_text(dajilog_motor_state_t state)
{
    switch (state) {
    case DAJILOG_MOTOR_OFFLINE: return "OFFLINE";
    case DAJILOG_MOTOR_READY: return "READY";
    case DAJILOG_MOTOR_RUNNING: return "RUN";
    case DAJILOG_MOTOR_STOPPING: return "STOPPING";
    case DAJILOG_MOTOR_FAULT: return "FAULT";
    default: return "DISABLED";
    }
}

/** @brief [Dashboard] 운전·CAN·속도·모터 1~4·설정 카드 상태를 Snapshot으로 표시한다. */
void render_dashboard(const dajilog_dashboard_snapshot_t &snapshot)
{
    /* [D-001] UI는 System Task가 보존한 자동 정지 원인과 출력 확인만 표시한다. */
    if (snapshot.auto_stop_notice_sequence != 0U) {
        set_string(FLOW_GLOBAL_VARIABLE_AUTO_STOP_REASON_TEXT,
                   s_auto_stop_reason_text, sizeof(s_auto_stop_reason_text),
                   STRING_ID_DASHBOARD_BASE + 8,
                   snapshot.auto_stop_reason_text);
        set_string(FLOW_GLOBAL_VARIABLE_AUTO_STOP_DETAIL_TEXT,
                   s_auto_stop_detail_text, sizeof(s_auto_stop_detail_text),
                   STRING_ID_DASHBOARD_BASE + 9,
                   snapshot.auto_stop_zero_confirmed
                       ? "출력이 정지되었습니다. 원인을 확인한 뒤 다시 시작하세요."
                       : "출력 정지를 확인하는 중입니다.");
        set_string(FLOW_GLOBAL_VARIABLE_AUTO_STOP_ZERO_CONFIRM_TEXT,
                   s_auto_stop_zero_text, sizeof(s_auto_stop_zero_text),
                   STRING_ID_DASHBOARD_BASE + 10,
                   snapshot.auto_stop_zero_confirmed
                       ? "모터 출력 0V 확인" : "모터 출력 0V 확인 대기");
        eez::flow::setGlobalVariable(
            FLOW_GLOBAL_VARIABLE_AUTO_STOP_NOTICE_SEQUENCE,
            eez::IntegerValue(static_cast<int32_t>(snapshot.auto_stop_notice_sequence)));
        eez::flow::setGlobalVariable(
            FLOW_GLOBAL_VARIABLE_AUTO_STOP_NOTICE_UNCONFIRMED,
            eez::BooleanValue(!snapshot.auto_stop_zero_confirmed &&
                              snapshot.operation_state == DAJILOG_OPERATION_FAULT));
        if (s_auto_stop_presented_sequence != snapshot.auto_stop_notice_sequence) {
            s_auto_stop_presented_sequence = snapshot.auto_stop_notice_sequence;
            eez::flow::setGlobalVariable(
                FLOW_GLOBAL_VARIABLE_AUTO_STOP_NOTICE_VISIBLE,
                eez::BooleanValue(
                    snapshot.auto_stop_notice_sequence != s_auto_stop_acknowledged_sequence));
        }
    }
    set_string(FLOW_GLOBAL_VARIABLE_OPERATION_STATE_TEXT,
               s_operation_text, sizeof(s_operation_text),
               STRING_ID_DASHBOARD_BASE + 0, operation_text(snapshot.operation_state));
    /* 운전 중에는 STOP, 완전히 정지한 READY에서는 추정 작업량 초기화. */
    if (objects.dashboard_setting_label && objects.dashboard_setting_description) {
        const bool ready = snapshot.operation_state == DAJILOG_OPERATION_READY;
        const char *title = ready ? "초기화" : "STOP";
        const char *description = ready ? "추정 작업량 0으로" : "즉시 전체 정지";
        if (std::strcmp(lv_label_get_text(objects.dashboard_setting_label), title) != 0)
            lv_label_set_text(objects.dashboard_setting_label, title);
        if (std::strcmp(lv_label_get_text(objects.dashboard_setting_description), description) != 0)
            lv_label_set_text(objects.dashboard_setting_description, description);
    }
    set_string(FLOW_GLOBAL_VARIABLE_CAN_ONLINE_TEXT,
               s_can_text, sizeof(s_can_text), STRING_ID_DASHBOARD_BASE + 1,
               snapshot.can_online ? "ONLINE" : "OFFLINE");
    set_text(FLOW_GLOBAL_VARIABLE_VEHICLE_SPEED_KMH_TEXT,
             s_vehicle_speed_text, sizeof(s_vehicle_speed_text),
             STRING_ID_DASHBOARD_BASE + 2, "%.1f", snapshot.vehicle.speed_kmh);
    std::snprintf(s_vehicle_target_text, sizeof(s_vehicle_target_text),
                  "%.1f km/h", static_cast<double>(snapshot.vehicle.target_speed_kmh));
    eez::flow::setGlobalVariable(
        FLOW_GLOBAL_VARIABLE_VEHICLE_TARGET_SPEED_KMH_TEXT,
        eez::Value::makeStringRef(s_vehicle_target_text, -1,
                                  STRING_ID_DASHBOARD_BASE + 3));
    if (snapshot.vehicle.simulated) {
        std::snprintf(s_vehicle_signal_text, sizeof(s_vehicle_signal_text),
                      "SIM · 1-2-3-4-3-2 km/h");
    } else if (snapshot.vehicle.online) {
        std::snprintf(s_vehicle_signal_text, sizeof(s_vehicle_signal_text),
                      "정상 · %lu ms", (unsigned long)snapshot.vehicle.signal_age_ms);
    } else {
        std::snprintf(s_vehicle_signal_text, sizeof(s_vehicle_signal_text), "신호 없음");
    }
    eez::flow::setGlobalVariable(
        FLOW_GLOBAL_VARIABLE_VEHICLE_SPEED_SIGNAL_TEXT,
        eez::Value::makeStringRef(s_vehicle_signal_text, -1,
                                  STRING_ID_DASHBOARD_BASE + 4));
    set_string(FLOW_GLOBAL_VARIABLE_WORK_MODE_TEXT,
               s_work_mode_text, sizeof(s_work_mode_text),
               STRING_ID_DASHBOARD_BASE + 5, work_mode_text(snapshot.work_mode));
    set_string(FLOW_GLOBAL_VARIABLE_RECIPE_NAME_TEXT,
               s_recipe_name_text, sizeof(s_recipe_name_text),
               STRING_ID_DASHBOARD_BASE + 6, snapshot.recipe_name);
    set_string(FLOW_GLOBAL_VARIABLE_DASHBOARD_ALERT_TEXT,
               s_alarm_text, sizeof(s_alarm_text), STRING_ID_DASHBOARD_BASE + 7,
               snapshot.alarm_text[0] != '\0' ? snapshot.alarm_text : "정상");

    const uint32_t driver_state_vars[] = {
        FLOW_GLOBAL_VARIABLE_DRIVER_1_STATE_TEXT,
        FLOW_GLOBAL_VARIABLE_DRIVER_2_STATE_TEXT,
    };
    const uint32_t driver_speed_vars[] = {
        FLOW_GLOBAL_VARIABLE_DRIVER_1_SPEED_RPM_TEXT,
        FLOW_GLOBAL_VARIABLE_DRIVER_2_SPEED_RPM_TEXT,
    };
    const uint32_t driver_target_vars[] = {
        FLOW_GLOBAL_VARIABLE_DRIVER_1_TARGET_KG_MIN_TEXT,
        FLOW_GLOBAL_VARIABLE_DRIVER_2_TARGET_KG_MIN_TEXT,
    };
    const uint32_t driver_load_vars[] = {
        FLOW_GLOBAL_VARIABLE_DRIVER_1_LOAD_PCT_TEXT,
        FLOW_GLOBAL_VARIABLE_DRIVER_2_LOAD_PCT_TEXT,
    };
    const uint32_t setting_state_vars[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_1_STATE_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_2_STATE_TEXT,
    };
    const uint32_t setting_name_vars[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_1_NAME_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_2_NAME_TEXT,
    };
    const uint32_t setting_target_vars[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_1_TARGET_KG_MIN_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_2_TARGET_KG_MIN_TEXT,
    };
    const uint32_t setting_load_vars[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_1_LOAD_PCT_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_2_LOAD_PCT_TEXT,
    };
    for (size_t i = 0; i < 2U; ++i) {
        const auto &motor = snapshot.motors[i * 2U];
        const auto &paired_motor = snapshot.motors[i * 2U + 1U];
        set_string(driver_state_vars[i], s_motor_state_text[i], sizeof(s_motor_state_text[i]),
                   STRING_ID_DASHBOARD_BASE + 10 + i * 4,
                   motor_state_text(motor.state));
        std::snprintf(s_motor_speed_text[i], sizeof(s_motor_speed_text[i]), "%lu",
                      (unsigned long)motor.actual_rpm);
        std::snprintf(s_motor_target_text[i], sizeof(s_motor_target_text[i]), "%.3f",
                      static_cast<double>(motor.target_kg_min + paired_motor.target_kg_min));
        std::snprintf(s_motor_load_text[i], sizeof(s_motor_load_text[i]), "%.0f%%",
                      static_cast<double>(motor.load_pct));
        eez::flow::setGlobalVariable(driver_speed_vars[i], eez::Value::makeStringRef(
            s_motor_speed_text[i], -1, STRING_ID_DASHBOARD_BASE + 11 + i * 4));
        eez::flow::setGlobalVariable(driver_target_vars[i], eez::Value::makeStringRef(
            s_motor_target_text[i], -1, STRING_ID_DASHBOARD_BASE + 12 + i * 4));
        eez::flow::setGlobalVariable(driver_load_vars[i], eez::Value::makeStringRef(
            s_motor_load_text[i], -1, STRING_ID_DASHBOARD_BASE + 13 + i * 4));

        set_string(setting_state_vars[i], s_motor_state_text[i + 2U],
                   sizeof(s_motor_state_text[i + 2U]),
                   STRING_ID_DASHBOARD_BASE + 30 + i * 4,
                   motor.state == DAJILOG_MOTOR_OFFLINE ? "OFFLINE" : "READY");
        set_string(setting_name_vars[i], s_setting_name_text[i],
                   sizeof(s_setting_name_text[i]),
                   STRING_ID_DASHBOARD_BASE + 31 + i * 4,
                   snapshot.recipe_name);
        eez::flow::setGlobalVariable(setting_target_vars[i], eez::Value::makeStringRef(
            s_motor_target_text[i], -1, STRING_ID_DASHBOARD_BASE + 32 + i * 4));
        eez::flow::setGlobalVariable(setting_load_vars[i], eez::Value::makeStringRef(
            s_motor_load_text[i], -1, STRING_ID_DASHBOARD_BASE + 33 + i * 4));
    }
    const uint32_t channel_vars[] = {
        FLOW_GLOBAL_VARIABLE_DRIVER_1_CHANNEL_A_SUMMARY_TEXT,
        FLOW_GLOBAL_VARIABLE_DRIVER_1_CHANNEL_B_SUMMARY_TEXT,
        FLOW_GLOBAL_VARIABLE_DRIVER_2_CHANNEL_C_SUMMARY_TEXT,
        FLOW_GLOBAL_VARIABLE_DRIVER_2_CHANNEL_D_SUMMARY_TEXT,
    };
    for (size_t i = 0; i < DAJILOG_MOTOR_CHANNEL_COUNT; ++i) {
        const auto &motor = snapshot.motors[i];
        const auto &driver = snapshot.drivers[i / 2];
        const unsigned target = driver.mode == 1
            ? motor.command_decivolt
            : driver.mode == 2
                ? driver.step[i % 2] * 5U + driver.fine_decivolt[i % 2] : 0;
        char feedback[16] = "--.-";
        if (motor.online && motor.measured_voltage_valid &&
            esp_timer_get_time() - motor.measured_timestamp_us < 3000000)
            std::snprintf(feedback, sizeof(feedback), "%.1f", motor.measured_decivolt / 10.0);
        std::snprintf(s_driver_summary_text[i], sizeof(s_driver_summary_text[i]),
                      "%.1f / %s", target / 10.0, feedback);
        set_string(channel_vars[i], s_driver_summary_text[i],
                   sizeof(s_driver_summary_text[i]), STRING_ID_DASHBOARD_BASE + 50U + i,
                   s_driver_summary_text[i]);
    }
    const uint32_t mode_vars[] = {FLOW_GLOBAL_VARIABLE_DRIVER_1_STATE_TEXT,
                                  FLOW_GLOBAL_VARIABLE_DRIVER_2_STATE_TEXT};
    const uint32_t status_vars[] = {FLOW_GLOBAL_VARIABLE_DRIVER_1_OUTPUT_STATE_TEXT,
                                    FLOW_GLOBAL_VARIABLE_DRIVER_2_OUTPUT_STATE_TEXT};
    for (size_t d = 0; d < 2; ++d) {
        set_string(mode_vars[d], s_motor_state_text[d], sizeof(s_motor_state_text[d]),
                   STRING_ID_DASHBOARD_BASE + 70U + d,
                   snapshot.drivers[d].mode == 2 ? "수동" :
                   snapshot.drivers[d].mode == 1 ? "자동" : "사용 안 함");
        const auto &a = snapshot.motors[d * 2], &b = snapshot.motors[d * 2 + 1];
        set_string(status_vars[d], s_driver_output_state_text[d], sizeof(s_driver_output_state_text[d]),
                   STRING_ID_DASHBOARD_BASE + 54U + d,
                   !a.online || !b.online ? "통신 오류" :
                   a.error_code || b.error_code ? "출력 오류" :
                   !a.measured_voltage_valid || !b.measured_voltage_valid ? "피드백 없음" :
                   snapshot.operation_state == DAJILOG_OPERATION_READY ? "정지" : "출력 중");
    }
    render_application_estimate(snapshot);
    render_detail_voltage();
}

struct CalibrationPreview {
    double total_kg_min;
    uint32_t target_g_min[2];
    uint32_t expected_g_min[2];
    uint16_t voltage_decivolt[2];
    bool table_found[2];
    bool in_range[2];
};

/** @brief [설정 카드 계산] A/B 또는 C/D 전압 미리보기만 계산하며 모터 명령은 보내지 않는다. */
CalibrationPreview calculate_calibration_preview(
    const dajilog_product_settings_snapshot_t &settings, size_t zone_index)
{
    CalibrationPreview preview = {};
    dajilog_ridge_profiles_snapshot_t ridges = {};
    if (zone_index >= DAJILOG_SETTING_ZONE_COUNT ||
        dajilog_system_core_get_ridge_profiles(&ridges) != ESP_OK) return preview;
    dajilog_flow_target_t flow = {};
    if (dajilog_flow_calculate_preview(&ridges, &settings.zones[zone_index], &flow) !=
        DAJILOG_FLOW_CALC_OK) return preview;
    preview.total_kg_min = flow.total_kg_min;
    preview.target_g_min[0] = flow.channel_target_g_min[0];
    preview.target_g_min[1] = flow.channel_target_g_min[1];
    const char *product = settings.selected_product_codes[zone_index];
    if (product[0] == '\0') return preview;
    for (size_t i = 0; i < 2U; ++i) {
        const char channel = static_cast<char>('A' + zone_index * 2U + i);
        dajilog_calibration_summary_t table = {};
        const auto &binding = settings.calibration[zone_index * 2U + i];
        if (binding.table_id[0] != '\0') {
            if (dajilog_calibration_catalog_find(
                    channel, binding.table_id, binding.revision, &table) != ESP_OK ||
                table.content_hash != binding.content_hash ||
                std::strcmp(table.product_code, product) != 0 ||
                !table.approved) continue;
        } else if (dajilog_calibration_catalog_list(
                       channel, product, "", &table, 1U) == 0U ||
                   !table.approved) continue;
        preview.table_found[i] = true;
        preview.in_range[i] = dajilog_calibration_catalog_preview_voltage(
            channel, table.table_id, table.revision, preview.target_g_min[i],
            &preview.voltage_decivolt[i], &preview.expected_g_min[i]) == ESP_OK;
    }
    return preview;
}

/** @brief [Dashboard 설정 카드·SettingDetail] S1/S2 설정과 계산 미리보기를 표시한다. */
void render_setting_views(bool include_detail)
{
    dajilog_product_settings_snapshot_t draft = {};
    dajilog_product_settings_snapshot_t applied = {};
    if (dajilog_system_core_get_settings_draft(&draft) != ESP_OK) return;
    const bool has_applied =
        dajilog_system_core_get_settings_applied(&applied) == ESP_OK;
    dajilog_dashboard_snapshot_t live = {};
    const bool has_live = dajilog_system_core_get_snapshot(&live) == ESP_OK;
    const uint32_t name_vars[2] = {
        FLOW_GLOBAL_VARIABLE_SURFACE_SETTING_NAME_TEXT,
        FLOW_GLOBAL_VARIABLE_SUBSURFACE_SETTING_NAME_TEXT,
    };
    const uint32_t state_vars[2] = {
        FLOW_GLOBAL_VARIABLE_SURFACE_SETTING_STATE_TEXT,
        FLOW_GLOBAL_VARIABLE_SUBSURFACE_SETTING_STATE_TEXT,
    };
    const uint32_t product_vars[2] = {
        FLOW_GLOBAL_VARIABLE_SURFACE_PRODUCT_LOT_TEXT,
        FLOW_GLOBAL_VARIABLE_SUBSURFACE_PRODUCT_LOT_TEXT,
    };
    const uint32_t target_vars[2] = {
        FLOW_GLOBAL_VARIABLE_SURFACE_TARGET_KG_MIN_TEXT,
        FLOW_GLOBAL_VARIABLE_SUBSURFACE_TARGET_KG_MIN_TEXT,
    };
    const uint32_t table_vars[2] = {
        FLOW_GLOBAL_VARIABLE_SURFACE_CALIBRATION_STATE_TEXT,
        FLOW_GLOBAL_VARIABLE_SUBSURFACE_CALIBRATION_STATE_TEXT,
    };
    for (size_t i = 0; i < 2; ++i) {
        const bool active = has_applied && applied.zones[i].configured;
        const auto &zone = active ? applied.zones[i] : draft.zones[i];
        const char *state = active ? "적용됨" :
            draft.zones[i].configured && draft.zones[i].rate_kg_per_10a <= 0.0f
                ? "처방 계산 대기" :
            draft.zones[i].configured
                ? (draft.dirty ? "편집 중 · 미적용" : "저장됨 · 미적용")
                : draft.zones[i].name[0] ? "편집 중 · 입력 미완료"
                : "미설정";
        char label[96];
        setting_text(name_vars[i], zone.name[0] ? zone.name : "미설정");
        setting_text(state_vars[i], state);
        std::snprintf(label, sizeof(label), "%s%s%s",
                      zone.product[0] ? zone.product : "미설정",
                      zone.lot[0] ? " / " : "", zone.lot);
        setting_text(product_vars[i], label);
        const CalibrationPreview preview = calculate_calibration_preview(draft, i);
        if (preview.total_kg_min > 0.0) {
            std::snprintf(label, sizeof(label), "%.2f", preview.total_kg_min);
        setting_text(target_vars[i], label);
        } else setting_text(target_vars[i], "--");
        if (has_live && live.operation_state == DAJILOG_OPERATION_RUN_AUTO &&
            live.drivers[i].mode == 1) {
            std::snprintf(label, sizeof(label), "%.3f",
                static_cast<double>(live.motors[i * 2U].target_kg_min +
                                    live.motors[i * 2U + 1U].target_kg_min));
            setting_text(target_vars[i], label);
        }
        setting_text(table_vars[i], preview.table_found[0] && preview.table_found[1]
                                       ? "계산용 표 연결" : "배출량표 없음");
        setting_text(i == 0 ? FLOW_GLOBAL_VARIABLE_SETTING_1_STATE_TEXT :
                              FLOW_GLOBAL_VARIABLE_SETTING_2_STATE_TEXT, state);
        setting_text(i == 0 ? FLOW_GLOBAL_VARIABLE_SETTING_1_NAME_TEXT :
                              FLOW_GLOBAL_VARIABLE_SETTING_2_NAME_TEXT,
                     zone.name[0] ? zone.name : "미설정");
        setting_text(i == 0 ? FLOW_GLOBAL_VARIABLE_SETTING_1_TARGET_KG_MIN_TEXT :
                              FLOW_GLOBAL_VARIABLE_SETTING_2_TARGET_KG_MIN_TEXT,
                     (has_live && live.operation_state == DAJILOG_OPERATION_RUN_AUTO &&
                      live.drivers[i].mode == 1) || preview.total_kg_min > 0.0
                         ? label : "--");
    }

    if (!include_detail) return;

    const size_t selected = static_cast<size_t>(s_selected_zone);
    const auto &zone = has_applied && applied.zones[selected].configured
                           ? applied.zones[selected] : draft.zones[selected];
    char value[96];
    const bool editable = [&] {
        dajilog_dashboard_snapshot_t snapshot = {};
        return dajilog_system_core_get_snapshot(&snapshot) == ESP_OK &&
               snapshot.operation_state == DAJILOG_OPERATION_READY;
    }();
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_SELECTED_ZONE,
                 selected == 0 ? "SURFACE" : "SUBSURFACE");
    eez::flow::setGlobalVariable(
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_HAS_DRAFT,
        eez::BooleanValue(draft.dirty));
    eez::flow::setGlobalVariable(
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CAN_EDIT,
        eez::BooleanValue(editable));
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_TITLE_TEXT,
                 selected == 0 ? "S1 전층 설정" : "S2 지중 설정");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_NAME_TEXT,
                 zone.name[0] ? zone.name : "미설정");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_APPLY_STATE_TEXT,
                 has_applied && applied.zones[selected].configured ? "적용됨" :
                 zone.configured && zone.rate_kg_per_10a <= 0.0f ? "처방 계산 대기" :
                 zone.configured ? "저장/적용 대기" :
                 zone.name[0] ? "입력 미완료" : "미설정");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_PRODUCT_TEXT,
                 zone.product[0] ? zone.product : "미설정");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_LOT_TEXT,
                 zone.lot[0] ? zone.lot : "--");
    std::snprintf(value, sizeof(value), "%s%s%s",
                  zone.product[0] ? zone.product : "미설정",
                  zone.lot[0] ? " / " : "", zone.lot);
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_PRODUCT_LOT_TEXT, value);
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_DRIVER_TEXT,
                 selected == 0 ? "D1 · A/B" : "D2 · C/D");
    std::snprintf(value, sizeof(value), "%c %u%% / %c %u%%",
                  selected == 0 ? 'A' : 'C',
                  zone.first_channel_permille / 10U,
                  selected == 0 ? 'B' : 'D',
                  (1000U - zone.first_channel_permille) / 10U);
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_DISTRIBUTION_TEXT,
                 zone.configured ? value : "--");
    std::snprintf(value, sizeof(value), "%.1f kg/10a",
                  static_cast<double>(zone.rate_kg_per_10a));
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_PRESCRIPTION_TEXT,
                 zone.configured && zone.rate_kg_per_10a > 0.0f ? value : "--");
    std::snprintf(value, sizeof(value), "%.1f km/h",
                  static_cast<double>(zone.reference_speed_kmh));
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_SPEED_TEXT,
                 zone.reference_speed_kmh > 0.0f ? value : "--");
    const CalibrationPreview preview = calculate_calibration_preview(draft, selected);
    /* 작업 전 확인: 선택 Bxx 한 구간의 길이와 처방 제품량으로 계획 준비량을 표시한다. */
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_RIDGE_SECTION_TEXT, "--");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CALCULATED_WIDTH_M_TEXT, "--");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_ACTUAL_LENGTH_M_TEXT, "--");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_PREPARATION_KG_TEXT, "--");
    dajilog_ridge_profiles_snapshot_t ridges = {};
    if (dajilog_system_core_get_ridge_profiles(&ridges) == ESP_OK) {
        for (uint32_t i = 0; i < ridges.count &&
                             i < DAJILOG_RIDGE_PROFILE_LIMIT; ++i) {
            const dajilog_ridge_profile_t &ridge = ridges.profiles[i];
            if (ridge.id != ridges.selected_id) continue;
            const auto &input = ridge.input;
            std::snprintf(value, sizeof(value), "B%02lu · %s",
                          static_cast<unsigned long>(ridge.id),
                          input.split_section ? "북측+남측" : "전체");
            setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_RIDGE_SECTION_TEXT, value);
            const double width = static_cast<double>(input.width_m) +
                (input.work_width_mode == DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW
                     ? static_cast<double>(input.furrow_width_m) : 0.0);
            const double length = input.split_section
                ? static_cast<double>(input.north_length_m) + input.south_length_m
                : static_cast<double>(input.whole_length_m);
            if (std::isfinite(width) && width > 0.0) {
                std::snprintf(value, sizeof(value), "%.2f", width);
                setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CALCULATED_WIDTH_M_TEXT, value);
            }
            if (std::isfinite(length) && length > 0.0) {
                std::snprintf(value, sizeof(value), "%.2f", length);
                setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_ACTUAL_LENGTH_M_TEXT, value);
            }
            if (std::isfinite(width) && width > 0.0 &&
                std::isfinite(length) && length > 0.0 &&
                zone.configured && std::isfinite(zone.rate_kg_per_10a) &&
                zone.rate_kg_per_10a > 0.0f) {
                const double preparation =
                    static_cast<double>(zone.rate_kg_per_10a) * width * length / 1000.0;
                std::snprintf(value, sizeof(value), "%.2f", preparation);
                setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_PREPARATION_KG_TEXT, value);
            }
            break;
        }
    }
    const uint32_t channel_target_vars[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_A_TARGET_G_MIN_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_B_TARGET_G_MIN_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_C_TARGET_G_MIN_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_D_TARGET_G_MIN_TEXT,
    };
    const uint32_t channel_voltage_vars[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_A_EXPECTED_COMMAND_VOLTAGE_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_B_EXPECTED_COMMAND_VOLTAGE_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_C_EXPECTED_COMMAND_VOLTAGE_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_D_EXPECTED_COMMAND_VOLTAGE_TEXT,
    };
    const uint32_t channel_table_vars[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_A_TABLE_LOOKUP_STATUS_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_B_TABLE_LOOKUP_STATUS_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_C_TABLE_LOOKUP_STATUS_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_D_TABLE_LOOKUP_STATUS_TEXT,
    };
    for (size_t i = 0; i < 4U; ++i) {
        setting_text(channel_target_vars[i], "--");
        setting_text(channel_voltage_vars[i], "--");
        setting_text(channel_table_vars[i], "표 없음");
    }
    for (size_t i = 0; i < 2U; ++i) {
        const size_t channel = selected * 2U + i;
        if (preview.total_kg_min > 0.0) {
            std::snprintf(value, sizeof(value), "%lu",
                          static_cast<unsigned long>(preview.target_g_min[i]));
            setting_text(channel_target_vars[channel], value);
        }
        setting_text(channel_table_vars[channel],
                     !preview.table_found[i] ? "표 없음" :
                     !preview.in_range[i] ? "범위 밖" : "조회 완료");
        if (preview.in_range[i]) {
            std::snprintf(value, sizeof(value), "%.1f",
                          preview.voltage_decivolt[i] / 10.0);
            setting_text(channel_voltage_vars[channel], value);
        }
    }
    if (preview.total_kg_min > 0.0) {
        std::snprintf(value, sizeof(value), "%.2f kg/min", preview.total_kg_min);
        setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_TOTAL_TARGET_TEXT, value);
    } else setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_TOTAL_TARGET_TEXT, "--");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_ORIGINAL_TOTAL_TEXT, "--");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_REFERENCE_TOTAL_TEXT, "--");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CALIBRATION_TEXT,
                 preview.table_found[0] && preview.table_found[1]
                     ? "계산용 표 연결" : "배출량표 없음");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_TABLE_STATUS_TEXT,
                 "전압 계산 미리보기 · CAN 미전송");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_VALIDATION_TEXT,
                 preview.in_range[0] && preview.in_range[1]
                     ? "A/B 또는 C/D 목표 전압 계산 완료" : "표 범위 또는 입력값 확인 필요");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_APPLY_BLOCK_TEXT,
                 "실제 모터 출력은 연결되지 않았습니다");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_1_NAME_TEXT,
                 selected == 0 ? "A" : "C");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_2_NAME_TEXT,
                 selected == 0 ? "B" : "D");
    const uint32_t missing_vars[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_1_TARGET_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_1_OUTPUT_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_1_VOLTAGE_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_2_TARGET_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_2_OUTPUT_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_2_VOLTAGE_TEXT,
    };
    for (uint32_t variable : missing_vars) setting_text(variable, "--");
    const uint32_t target_preview_vars[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_1_TARGET_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_2_TARGET_TEXT,
    };
    const uint32_t output_preview_vars[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_1_OUTPUT_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_2_OUTPUT_TEXT,
    };
    const uint32_t voltage_preview_vars[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_1_VOLTAGE_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_2_VOLTAGE_TEXT,
    };
    for (size_t i = 0; i < 2U; ++i) {
        if (preview.total_kg_min <= 0.0) continue;
        std::snprintf(value, sizeof(value), "%.3f", preview.target_g_min[i] / 1000.0);
        setting_text(target_preview_vars[i], value);
        if (!preview.in_range[i]) {
            setting_text(output_preview_vars[i], preview.table_found[i]
                         ? "범위 밖" : "표 없음");
            continue;
        }
        std::snprintf(value, sizeof(value), "%.3f", preview.expected_g_min[i] / 1000.0);
        setting_text(output_preview_vars[i], value);
        std::snprintf(value, sizeof(value), "%.1f", preview.voltage_decivolt[i] / 10.0);
        setting_text(voltage_preview_vars[i], value);
    }
    const uint32_t table_bases[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_A_STATE_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_B_STATE_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_C_STATE_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_D_STATE_TEXT,
    };
    for (uint32_t base : table_bases) {
        setting_text(base, "표 없음");
        for (uint32_t offset = 1; offset < 10; ++offset)
            setting_text(base + offset, "--");
    }
    const uint32_t table_bases_for_zone[] = {
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_A_STATE_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_B_STATE_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_C_STATE_TEXT,
        FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CHANNEL_D_STATE_TEXT,
    };
    char preview_reason[96] = {};
    for (size_t i = 0; i < 2U; ++i) {
        const size_t channel_index = selected * 2U + i;
        const uint32_t base = table_bases_for_zone[channel_index];
        const auto &binding = draft.calibration[channel_index];
        dajilog_calibration_summary_t table = {};
        const char channel = static_cast<char>('A' + channel_index);
        const bool linked = binding.table_id[0] &&
            dajilog_calibration_catalog_find(channel, binding.table_id,
                                              binding.revision, &table) == ESP_OK &&
            table.content_hash == binding.content_hash && table.approved &&
            std::strcmp(table.product_code, draft.selected_product_codes[selected]) == 0 &&
            (!draft.zones[selected].lot[0] ||
             std::strcmp(table.product_lot, draft.zones[selected].lot) == 0);
        setting_text(base, linked ? "연결됨" : binding.table_id[0] ? "연결 오류" : "선택 안 됨");
        setting_text(base + 3U, binding.table_id[0] ? binding.table_id : "--");
        std::snprintf(value, sizeof(value), "r%lu",
                      static_cast<unsigned long>(binding.revision));
        setting_text(base + 4U, binding.table_id[0] ? value : "--");
        if (!linked) {
            setting_text(base + 5U, "표·제품·Lot 확인");
            if (preview_reason[0] == '\0')
                std::snprintf(preview_reason, sizeof(preview_reason),
                              "%c: 표·제품·Lot 연결 확인 필요", channel);
        } else if (!preview.in_range[i]) {
            std::snprintf(value, sizeof(value), "목표 %lu / 범위 %lu–%lu g/min",
                          static_cast<unsigned long>(preview.target_g_min[i]),
                          static_cast<unsigned long>(table.min_discharge_g_min),
                          static_cast<unsigned long>(table.max_discharge_g_min));
            setting_text(base + 5U, value);
            if (preview_reason[0] == '\0')
                std::snprintf(preview_reason, sizeof(preview_reason), "%c: %.90s",
                              channel, value);
        } else {
            setting_text(base + 5U, "조회 완료");
        }
        if (preview.total_kg_min > 0.0) {
            std::snprintf(value, sizeof(value), "%lu g/min",
                          static_cast<unsigned long>(preview.target_g_min[i]));
            setting_text(base + 6U, value);
        }
        if (linked && preview.in_range[i]) {
            std::snprintf(value, sizeof(value), "%.1f V",
                          preview.voltage_decivolt[i] / 10.0);
            setting_text(base + 9U, value);
        }
    }
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_TABLE_STATUS_TEXT,
                 preview_reason[0] ? preview_reason :
                 "전압 계산 미리보기 · CAN 미전송");
    bool can_confirm = editable && !draft.dirty &&
        draft.saved_revision != 0U &&
        draft.saved_readiness == DAJILOG_SETTING_CONFIGURED;
    for (size_t i = 0; i < DAJILOG_MOTOR_CHANNEL_COUNT; ++i)
        can_confirm = can_confirm && draft.calibration[i].table_id[0] != '\0';
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_FOOTER_STATUS_TEXT,
                 s_setting_status[0] ? s_setting_status :
                 can_confirm ? "A~D 표 선택·저장 완료 · 운전에 적용 가능" :
                 editable ? "설정 저장과 A~D 배출량표 선택이 필요합니다" :
                            "운전 정지 후 변경할 수 있습니다");
    if (objects.setting_detail_apply_button) {
        if (can_confirm)
            lv_obj_remove_state(objects.setting_detail_apply_button, LV_STATE_DISABLED);
        else
            lv_obj_add_state(objects.setting_detail_apply_button, LV_STATE_DISABLED);
    }
    if (objects.setting_detail_info_card &&
        lv_obj_has_state(objects.setting_detail_info_card, LV_STATE_DISABLED) == editable)
        (editable ? lv_obj_remove_state : lv_obj_add_state)(
            objects.setting_detail_info_card, LV_STATE_DISABLED);
    if (objects.setting_detail_save_button &&
        lv_obj_has_state(objects.setting_detail_save_button, LV_STATE_DISABLED) == editable)
        (editable ? lv_obj_remove_state : lv_obj_add_state)(
            objects.setting_detail_save_button, LV_STATE_DISABLED);
    lv_obj_t *edit_controls[] = {
        objects.setting_edit_name_button,
        objects.setting_edit_product_button,
        objects.setting_edit_lot_button,
        objects.setting_edit_prescription_button,
        objects.setting_edit_speed_button,
        objects.setting_edit_distribution_button,
        objects.setting_edit_save_button,
    };
    for (lv_obj_t *control : edit_controls) {
        if (control && lv_obj_has_state(control, LV_STATE_DISABLED) == editable)
            (editable ? lv_obj_remove_state : lv_obj_add_state)(
                control, LV_STATE_DISABLED);
    }
    if (objects.setting_edit_prescription_button)
        lv_obj_add_state(objects.setting_edit_prescription_button, LV_STATE_DISABLED);
    if (objects.setting_edit_prescription_caption)
        lv_label_set_text(objects.setting_edit_prescription_caption, "처방 계산 제품량");
    if (has_live) render_application_estimate(live);
}

void render_latest_dashboard()
{
    dajilog_dashboard_snapshot_t snapshot = {};
    if (dajilog_system_core_get_snapshot(&snapshot) == ESP_OK) {
        render_dashboard(snapshot);
    }
}

/** @brief [모든 카드 공통] Service 결과를 LVGL Task에서 받아 해당 카드의 표시를 갱신한다. */
void render_message(const dajilog_ui_msg_t *message, void *)
{
    if (message == nullptr) {
        return;
    }
    if (message->topic == DAJILOG_UI_STATE_DASHBOARD_SNAPSHOT) {
        render_latest_dashboard();
    } else if (message->topic == DAJILOG_UI_STATE_RIDGE_RESULT &&
               message->sequence == s_ridge_request_sequence &&
               s_ridge_request_sequence != 0U) {
        s_ridge_request_sequence = 0U;
        if (message->result == DAJILOG_UI_RESULT_OK) {
            render_ridge_profiles(true);
            render_setting_views();
            if (s_ridge_pending_topic == DAJILOG_UI_CMD_RIDGE_DELETE) {
                dajilog_ridge_profiles_snapshot_t profiles = {};
                if (dajilog_system_core_get_ridge_profiles(&profiles) == ESP_OK &&
                    profiles.count == 0U) {
                    setting_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_WIDTH_M_TEXT, "");
                    setting_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_INCLUDED_FURROW_WIDTH_M_TEXT, "");
                    setting_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_ACTUAL_LENGTH_M_TEXT, "");
                    setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_NORTH_LENGTH_M_TEXT, "");
                    setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_SOUTH_LENGTH_M_TEXT, "");
                    render_ridge_preview();
                }
            }
            setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                         s_ridge_pending_topic == DAJILOG_UI_CMD_RIDGE_SAVE
                             ? "두둑 설정 NVS 저장 완료" :
                         s_ridge_pending_topic == DAJILOG_UI_CMD_RIDGE_DELETE
                             ? "두둑 Draft 삭제 · 설정 저장 필요" :
                         s_ridge_pending_topic == DAJILOG_UI_CMD_RIDGE_SELECT
                             ? "두둑 선택 완료" : "두둑 Draft 반영 · 설정 저장 필요");
        } else {
            setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                         s_ridge_pending_topic == DAJILOG_UI_CMD_RIDGE_SAVE
                             ? "두둑 NVS 저장 실패 · 다시 시도하세요" :
                         s_ridge_pending_topic == DAJILOG_UI_CMD_RIDGE_DELETE
                             ? "두둑 삭제 실패 · 기존 목록 유지" :
                               "두둑 등록/선택 실패 · 입력 또는 저장소 확인");
        }
        s_ridge_pending_topic = 0U;
    } else if (message->topic == DAJILOG_UI_STATE_COMMAND_RESULT &&
               s_run_request_sequence != 0U &&
               message->sequence == s_run_request_sequence) {
        s_run_request_sequence = 0U;
        if (message->result != DAJILOG_UI_RESULT_OK) {
            (void)render_run_readiness();
            eez::flow::replacePageHook(SCREEN_ID_RUN_READINESS, 0, 0, 0);
        }
    } else if (message->topic == DAJILOG_UI_STATE_COMMAND_RESULT &&
               message->sequence == s_ridge_request_sequence &&
               s_ridge_request_sequence != 0U) {
        s_ridge_request_sequence = 0U;
        s_ridge_pending_topic = 0U;
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "두둑 명령 거부 · 운전/시험 상태 확인");
    } else if (message->topic == DAJILOG_UI_STATE_COMMAND_RESULT &&
               s_setting_request_sequence != 0U &&
               message->sequence == s_setting_request_sequence) {
        s_setting_request_sequence = 0U;
        std::snprintf(s_setting_status, sizeof(s_setting_status),
                      "설정 명령 거부 · 운전/시험 상태를 확인하세요");
        render_setting_views();
    } else if (message->topic == DAJILOG_UI_STATE_COMMAND_RESULT) {
        if (message->sequence == s_apply_sequence && s_apply_sequence != 0) {
            s_apply_sequence = 0;
            if (message->result == DAJILOG_UI_RESULT_OK) {
                select_driver(s_detail_motor_index / 2U);
                lv_label_set_text(objects.driver_detail_safety_hint, "적용 완료 · 다음 RUN부터 사용 · 재부팅 시 초기화");
            } else {
                lv_label_set_text(objects.driver_detail_safety_hint, "적용 거부 · 정지 / CAN / Fault / 전압 범위를 확인하세요");
            }
        }
        if (message->result != DAJILOG_UI_RESULT_OK) {
            end_test();
            lv_label_set_text(objects.driver_detail_safety_hint,
                              "명령 거부 · 정지 상태와 CAN 전압/상태/Fault 응답을 확인하세요");
            ESP_LOGW(TAG, "product command rejected: result=%u", (unsigned)message->result);
        }
    } else if (message->topic == DAJILOG_UI_STATE_SETTINGS_RESULT &&
               s_setting_request_sequence != 0U &&
               message->sequence == s_setting_request_sequence) {
        s_setting_request_sequence = 0U;
        const bool confirmed = s_setting_request_topic == DAJILOG_UI_CMD_SETTINGS_CONFIRM;
        s_setting_request_topic = 0U;
        std::snprintf(s_setting_status, sizeof(s_setting_status), "%s",
                      message->result == DAJILOG_UI_RESULT_OK
                          ? confirmed ? "설정 확정 완료 · Applied 반영"
                                      : "설정 저장 완료 · 현재 운전에는 미적용"
                          : confirmed ? "설정 확정 실패 · 제품·Lot·A~D 표 확인"
                                      : "설정 저장 실패 · 입력과 저장소 확인");
        (void)render_current_recipe();
        render_setting_views();
        if (lv_screen_active() == objects.calibration_select) {
            std::snprintf(s_calibration_result, sizeof(s_calibration_result), "%s",
                          message->result == DAJILOG_UI_RESULT_OK
                              ? confirmed ? "Applied 설정 확정 완료" : "NVS 저장 완료 · Saved 갱신"
                              : "NVS 저장 실패 · 기존 Saved 유지");
            render_calibration_selection();
        }
    } else if (message->topic == DAJILOG_UI_STATE_SETTINGS_RESULT &&
               message->result == DAJILOG_UI_RESULT_OK) {
        (void)render_current_recipe();
        render_setting_views();
    }
}

esp_err_t submit_command(uint32_t topic)
{
    dajilog_ui_msg_t command;
    dajilog_ui_msg_init(&command, DAJILOG_UI_MSG_COMMAND, topic);
    return dajilog_ui_adaptor_submit(&command);
}

/** @brief [설정 카드 공통] Save/Confirm 요청을 Queue에 넣고 비동기 결과 번호를 기억한다. */
esp_err_t submit_setting_command(uint32_t topic)
{
    dajilog_ui_msg_t command;
    dajilog_ui_msg_init(&command, DAJILOG_UI_MSG_COMMAND, topic);
    const esp_err_t error = dajilog_ui_adaptor_submit(&command);
    if (error == ESP_OK) {
        s_setting_request_sequence = command.sequence;
        s_setting_request_topic = topic;
        s_setting_request_deadline = esp_timer_get_time() + 5000000;
        std::snprintf(s_setting_status, sizeof(s_setting_status),
                      "설정 처리 중...");
        render_setting_views();
    }
    return error;
}

/** @brief [DriverDetail 모터 카드] 단계·보정값을 합쳐 화면에 표시할 목표 전압을 만든다. */
uint16_t detail_final_decivolt()
{
    const int32_t value =
        (int32_t)s_detail_output_step * DAJILOG_MOTOR_DECIVOLT_PER_STEP +
        s_detail_trim_decivolt;
    return static_cast<uint16_t>(value < 0 ? 0 :
        (value > DAJILOG_MOTOR_OUTPUT_DECIVOLT_MAX
             ? static_cast<int32_t>(DAJILOG_MOTOR_OUTPUT_DECIVOLT_MAX) : value));
}

/** @brief [DriverDetail 모터 카드] 목표·현재 전압과 단계 조작 상태를 다시 그린다. */
void render_detail_voltage()
{
    if (objects.driver_detail_trim_apply_label != nullptr) {
        lv_label_set_text(objects.driver_detail_trim_apply_label,
                          s_detail_service_test ? "누르는 동안" : "적용");
        lv_label_set_text(objects.driver_detail_trim_reset_label,
                          s_detail_service_test ? "즉시 정지" : "취소");
    }
    const uint16_t final_decivolt = detail_final_decivolt();
    s_driver_draft.step[s_detail_motor_index % 2] = s_detail_output_step;
    s_driver_draft.fine_decivolt[s_detail_motor_index % 2] = s_detail_trim_decivolt;
    set_string(FLOW_GLOBAL_VARIABLE_DRIVER_DETAIL_DRAFT_MODE_TEXT,
               s_draft_mode_text, sizeof(s_draft_mode_text), STRING_ID_MOTOR_VOLTAGE_BASE + 9,
               s_detail_service_test ? "출력 시험" : s_driver_draft.mode == 1 ? "자동" : "수동");
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_DRIVER_MODE_AUTO_SELECTED, eez::BooleanValue(!s_detail_service_test && s_driver_draft.mode == 1));
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_DRIVER_MODE_MANUAL_SELECTED, eez::BooleanValue(!s_detail_service_test && s_driver_draft.mode == 2));
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_DRIVER_MODE_DISABLED_SELECTED, eez::BooleanValue(s_detail_service_test));
    std::snprintf(s_detail_step_text, sizeof(s_detail_step_text), "%u",
                  static_cast<unsigned>(s_detail_output_step));
    std::snprintf(s_detail_final_voltage_text,
                  sizeof(s_detail_final_voltage_text), "%u.%u V",
                  final_decivolt / 10U, final_decivolt % 10U);
    std::snprintf(s_detail_target_summary_text,
                  sizeof(s_detail_target_summary_text),
                  "기본 %u.%u V · 보정 %+.1f V",
                  (s_detail_output_step * 5U) / 10U,
                  (s_detail_output_step * 5U) % 10U,
                  static_cast<double>(s_detail_trim_decivolt) / 10.0);
    dajilog_dashboard_snapshot_t snapshot = {};
    const auto *motor = dajilog_system_core_get_snapshot(&snapshot) == ESP_OK
                            ? &snapshot.motors[s_detail_motor_index] : nullptr;
    const auto &applied = snapshot.drivers[s_detail_motor_index / 2];
    const unsigned current = applied.step[s_detail_motor_index % 2] * 5U +
                             applied.fine_decivolt[s_detail_motor_index % 2];
    std::snprintf(s_detail_voltage_summary_text, sizeof(s_detail_voltage_summary_text),
                  "현재 설정 %.1f → 예정 %.1f V / 변경 %+.1f V",
                  current / 10.0, final_decivolt / 10.0,
                  (static_cast<int>(final_decivolt) - static_cast<int>(current)) / 10.0);
    (void)motor;
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_DETAIL_OUTPUT_STEP,
                                 eez::IntegerValue(s_detail_output_step));
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_DETAIL_OUTPUT_DECIVOLT,
                                 eez::IntegerValue(final_decivolt));
    set_string(FLOW_GLOBAL_VARIABLE_DETAIL_OUTPUT_STEP_TEXT, s_detail_step_text,
               sizeof(s_detail_step_text), STRING_ID_MOTOR_VOLTAGE_BASE + 1,
               s_detail_step_text);
    set_string(FLOW_GLOBAL_VARIABLE_DETAIL_FINAL_VOLTAGE_TEXT,
               s_detail_final_voltage_text, sizeof(s_detail_final_voltage_text),
               STRING_ID_MOTOR_VOLTAGE_BASE + 2, s_detail_final_voltage_text);
    set_string(FLOW_GLOBAL_VARIABLE_DETAIL_TARGET_SUMMARY_TEXT,
               s_detail_target_summary_text, sizeof(s_detail_target_summary_text),
               STRING_ID_MOTOR_VOLTAGE_BASE + 3, s_detail_target_summary_text);
    set_string(FLOW_GLOBAL_VARIABLE_DETAIL_VOLTAGE_SUMMARY_TEXT,
               s_detail_voltage_summary_text, sizeof(s_detail_voltage_summary_text),
               STRING_ID_MOTOR_VOLTAGE_BASE + 4, s_detail_voltage_summary_text);
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_DETAIL_OUTPUT_SAFE,
                                 eez::BooleanValue(final_decivolt <= 120U));
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_DETAIL_APPLY_ACTIVE,
                                 eez::BooleanValue(s_driver_draft.mode == 1 || s_driver_draft.mode == 2 || s_detail_service_test));
}

/** @brief [DriverDetail 모터 카드] 선택 모터 드라이버의 Draft를 상세 화면에 불러온다. */
void select_driver(size_t driver)
{
    s_detail_motor_index = static_cast<uint8_t>(driver * 2U);
    dajilog_dashboard_snapshot_t snapshot = {};
    (void)dajilog_system_core_get_snapshot(&snapshot);
    s_driver_draft = snapshot.drivers[driver];
    s_driver_draft.driver_index = driver;
    if (s_driver_draft.mode == 0) s_driver_draft.mode = 2;
    s_detail_output_step = s_driver_draft.step[0];
    s_detail_trim_decivolt = s_driver_draft.fine_decivolt[0];
    s_detail_service_test = false;
    set_string(FLOW_GLOBAL_VARIABLE_DETAIL_SELECTED_DRIVER_TEXT,
               s_detail_selected_driver_text, sizeof(s_detail_selected_driver_text),
               STRING_ID_MOTOR_VOLTAGE_BASE + 5,
               driver == 0U ? "D1 · 전층 드라이버" : "D2 · 심층 드라이버");
    set_string(FLOW_GLOBAL_VARIABLE_DETAIL_SELECTED_CHANNEL_TEXT,
               s_detail_selected_channel_text, sizeof(s_detail_selected_channel_text),
               STRING_ID_MOTOR_VOLTAGE_BASE + 6, driver == 0U ? "A" : "C");
    if (objects.driver_detail_channel_a_selector_label != nullptr) {
        lv_label_set_text(objects.driver_detail_channel_a_selector_label,
                          driver == 0U ? "A" : "C");
        lv_label_set_text(objects.driver_detail_channel_b_selector_label,
                          driver == 0U ? "B" : "D");
    }
    render_detail_voltage();
}

/** @brief Maintain the driver editor draft and submit requests without direct hardware access. */
void driver_card_event(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_PRESSED) {
        end_test();
        select_driver((size_t)(uintptr_t)lv_event_get_user_data(event));
        eez::flow::replacePageHook(SCREEN_ID_DRIVER_DETAIL, 0, 0, 0);
    }
}

/** @brief Maintain the driver editor draft and submit requests without direct hardware access. */
void detail_channel_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    end_test();
    const size_t local = (size_t)(uintptr_t)lv_event_get_user_data(event);
    s_detail_motor_index = (uint8_t)((s_detail_motor_index / 2U) * 2U + local);
    s_detail_output_step = s_driver_draft.step[local];
    s_detail_trim_decivolt = s_driver_draft.fine_decivolt[local];
    const char channel = static_cast<char>('A' + s_detail_motor_index);
    char value[2] = {channel, '\0'};
    set_string(FLOW_GLOBAL_VARIABLE_DETAIL_SELECTED_CHANNEL_TEXT,
               s_detail_selected_channel_text, sizeof(s_detail_selected_channel_text),
               STRING_ID_MOTOR_VOLTAGE_BASE + 6, value);
    render_detail_voltage();
}

/** @brief Select a draft mode; changing tabs never applies or energizes outputs. */
void detail_mode_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    end_test();
    const uintptr_t mode = (uintptr_t)lv_event_get_user_data(event);
    s_detail_service_test = mode == 3;
    if (!s_detail_service_test) s_driver_draft.mode = mode;
    lv_label_set_text(objects.driver_detail_trim_apply_label, s_detail_service_test ? "누르는 동안" : "적용");
    lv_label_set_text(objects.driver_detail_trim_reset_label, s_detail_service_test ? "즉시 정지" : "취소");
    lv_label_set_text(objects.driver_detail_safety_hint, mode == 1
        ? "시험용 AUTO · 적용된 설정과 전진 속도가 필요합니다"
        : mode == 3 ? "정지 상태에서 한 채널만 시험 · 손을 떼면 0V"
                    : "채널별 0~24단계 +0.0~0.4V · 적용은 출력하지 않습니다");
    render_detail_voltage();
}

/** @brief Maintain the driver editor draft and submit requests without direct hardware access. */
void detail_slider_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_VALUE_CHANGED) return;
    end_test();
    int32_t value = lv_slider_get_value(lv_event_get_target_obj(event));
    s_detail_output_step = static_cast<uint8_t>(value < 0 ? 0 :
        (value > 24 ? 24 : value));
    if (s_detail_output_step == 24) s_detail_trim_decivolt = 0;
    render_detail_voltage();
}

/** @brief Maintain the driver editor draft and submit requests without direct hardware access. */
void detail_trim_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    end_test();
    const intptr_t delta = (intptr_t)lv_event_get_user_data(event);
    if (delta == 0) s_detail_trim_decivolt = 0;
    else if (delta < 0 && s_detail_trim_decivolt > 0) --s_detail_trim_decivolt;
    else if (delta > 0 && s_detail_trim_decivolt < 4 && s_detail_output_step < 24) ++s_detail_trim_decivolt;
    render_detail_voltage();
}

/** @brief Maintain the driver editor draft and submit requests without direct hardware access. */
esp_err_t submit_voltage_test(uint16_t final_decivolt)
{
    const dajilog_motor_voltage_command_t payload = {
        .motor_index = s_detail_motor_index,
        .output_step = s_detail_output_step,
        .trim_decivolt = s_detail_trim_decivolt,
        .final_decivolt = final_decivolt,
    };
    dajilog_ui_msg_t command;
    dajilog_ui_msg_init(&command, DAJILOG_UI_MSG_COMMAND,
                        DAJILOG_UI_CMD_MOTOR_VOLTAGE_TEST);
    command.payload.type = DAJILOG_UI_VALUE_BYTES;
    command.payload.value.bytes.size = sizeof(payload);
    std::memcpy(command.payload.value.bytes.data, &payload, sizeof(payload));
    return dajilog_ui_adaptor_submit(&command);
}

/** @brief Stop a hold session through the priority STOP path. */
void end_test()
{
    if (!s_test_held) return;
    s_test_held = false;
    (void)submit_command(DAJILOG_UI_CMD_STOP);
}

/** @brief Apply both draft channels, or keep a temporary hold-to-run lease alive. */
void detail_apply_event(lv_event_t *event)
{
    const auto code = lv_event_get_code(event);
    if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) { end_test(); return; }
    if (code != LV_EVENT_PRESSED) return;
    if (s_detail_service_test) {
        if (s_run_deadline != 0) return;
        s_test_held = submit_voltage_test(detail_final_decivolt()) == ESP_OK;
        s_test_refresh_us = esp_timer_get_time();
        return;
    }
    if (s_apply_sequence != 0) return;
    dajilog_ui_msg_t message;
    dajilog_ui_msg_init(&message, DAJILOG_UI_MSG_COMMAND, DAJILOG_UI_CMD_DRIVER_APPLY);
    message.sequence = ++s_sequence;
    message.payload.type = DAJILOG_UI_VALUE_BYTES;
    message.payload.value.bytes.size = sizeof(s_driver_draft);
    std::memcpy(message.payload.value.bytes.data, &s_driver_draft, sizeof(s_driver_draft));
    if (dajilog_ui_adaptor_submit(&message) == ESP_OK) {
        s_apply_sequence = message.sequence;
        s_apply_deadline = esp_timer_get_time() + 2000000;
    }
}

/** @brief Cancel editing, or stop a test without leaving its controls. */
void detail_cancel_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    if (s_detail_service_test) {
        end_test();
        (void)submit_command(DAJILOG_UI_CMD_STOP);
    } else {
        detail_close_event(event);
    }
}

/** @brief Discard local editing; service-test cancel is an immediate STOP. */
void detail_close_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    end_test();
    if (s_detail_service_test) (void)submit_command(DAJILOG_UI_CMD_STOP);
    select_driver(s_detail_motor_index / 2U);
    eez::flow::replacePageHook(SCREEN_ID_DASHBOARD, 0, 0, 0);
}

/** @brief Reload Applied values without leaving the editor. */
[[maybe_unused]] void detail_reset_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    end_test();
    select_driver(s_detail_motor_index / 2U);
}

/** @brief [D-004 RUN] 제품 Controller의 판정만 EEZ 체크리스트에 표시한다. */
bool render_run_readiness()
{
    dajilog_run_readiness_t result = {};
    if (dajilog_system_core_get_run_readiness(&result) != ESP_OK) {
        eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_RUN_READINESS_ALL_READY,
                                     eez::BooleanValue(false));
        eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_RUN_READINESS_OPERATION_STATE,
                                     eez::IntegerValue(DAJILOG_RUN_CHECK_FAIL));
        setting_text(FLOW_GLOBAL_VARIABLE_RUN_READINESS_OPERATION_REASON_TEXT,
                     "운전 조건 조회 실패");
        return false;
    }
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_RUN_READINESS_ALL_READY,
                                 eez::BooleanValue(result.all_ready));
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_RUN_READINESS_BUSY,
                                 eez::BooleanValue(false));
    char checked_at[32];
    std::snprintf(checked_at, sizeof(checked_at), "부팅 후 %llu초",
                  static_cast<unsigned long long>(result.checked_at_us / 1000000));
    setting_text(FLOW_GLOBAL_VARIABLE_RUN_READINESS_CHECKED_AT_TEXT, checked_at);
    setting_text(FLOW_GLOBAL_VARIABLE_RUN_READINESS_SUMMARY_TEXT,
                 result.all_ready ? "모든 조건이 확인됐습니다. Dashboard에서 RUN을 다시 누르세요." :
                 "미확인 조건을 수정한 뒤 다시 확인하세요. RUN은 Dashboard에서 시작합니다.");
    const uint32_t state_vars[DAJILOG_RUN_CHECK_COUNT] = {
        FLOW_GLOBAL_VARIABLE_RUN_READINESS_OPERATION_STATE,
        FLOW_GLOBAL_VARIABLE_RUN_READINESS_MOTOR_A_STATE,
        FLOW_GLOBAL_VARIABLE_RUN_READINESS_MOTOR_B_STATE,
        FLOW_GLOBAL_VARIABLE_RUN_READINESS_MOTOR_C_STATE,
        FLOW_GLOBAL_VARIABLE_RUN_READINESS_MOTOR_D_STATE,
        FLOW_GLOBAL_VARIABLE_RUN_READINESS_SPEED_STATE,
        FLOW_GLOBAL_VARIABLE_RUN_READINESS_APPLIED_STATE,
        FLOW_GLOBAL_VARIABLE_RUN_READINESS_PRODUCT_STATE,
        FLOW_GLOBAL_VARIABLE_RUN_READINESS_TABLE_A_STATE,
        FLOW_GLOBAL_VARIABLE_RUN_READINESS_TABLE_B_STATE,
        FLOW_GLOBAL_VARIABLE_RUN_READINESS_TABLE_C_STATE,
        FLOW_GLOBAL_VARIABLE_RUN_READINESS_TABLE_D_STATE,
        FLOW_GLOBAL_VARIABLE_RUN_READINESS_FAULT_STATE,
    };
    for (size_t i = 0; i < DAJILOG_RUN_CHECK_COUNT; ++i) {
        eez::flow::setGlobalVariable(state_vars[i],
            eez::IntegerValue(static_cast<int32_t>(result.items[i].state)));
        setting_text(state_vars[i] + 1U, result.items[i].reason);
    }
    return result.all_ready;
}

/** @brief [D-004 RUN] START 명령의 비동기 거부를 체크리스트 화면으로 되돌린다. */
esp_err_t submit_run_command(uint32_t topic)
{
    dajilog_ui_msg_t command;
    dajilog_ui_msg_init(&command, DAJILOG_UI_MSG_COMMAND, topic);
    const esp_err_t error = dajilog_ui_adaptor_submit(&command);
    if (error == ESP_OK) s_run_request_sequence = command.sequence;
    return error;
}

/** @brief [Dashboard 운전 카드] 준비조건이 모두 확인되면 3초 카운트다운을 시작한다. */
void run_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    if (s_test_held || !render_run_readiness()) {
        s_run_deadline = 0;
        eez::flow::replacePageHook(SCREEN_ID_RUN_READINESS, 0, 0, 0);
        return;
    }
    s_run_deadline = esp_timer_get_time() + 3000000;
    eez::flow::replacePageHook(SCREEN_ID_RUN_COUNTDOWN, 0, 0, 0);
}

/** @brief [D-004 RUN] 재확인은 조건만 조회하고 운전을 시작하지 않는다. */
void run_readiness_refresh_event(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_PRESSED) (void)render_run_readiness();
}

/** @brief Own driver-card callbacks; generated Flow remains responsible for layout/tick. */
void bind_driver_event(lv_obj_t *object, lv_event_cb_t callback, void *context = nullptr)
{
    while (lv_obj_get_event_count(object)) lv_obj_remove_event(object, 0);
    lv_obj_add_flag(object, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(object, callback, LV_EVENT_ALL, context);
}

/** @brief Add native handling while preserving EEZ Flow state/style callbacks. */
void bind_driver_selector_event(lv_obj_t *object, lv_event_cb_t callback, void *context)
{
    lv_obj_add_flag(object, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(object, callback, LV_EVENT_ALL, context);
}

/** @brief [입력 공통] EEZ 문자열을 정수로 검증·변환한다. */
bool parse_int(const char *text, int32_t *value)
{
    if (text == nullptr || value == nullptr || text[0] == '\0') {
        return false;
    }

    errno = 0;
    char *end = nullptr;
    const long parsed = std::strtol(text, &end, 10);
    if (errno == ERANGE || end == text || end == nullptr || *end != '\0' ||
        parsed < INT32_MIN || parsed > INT32_MAX) {
        return false;
    }
    *value = static_cast<int32_t>(parsed);
    return true;
}

/** @brief [입력 공통] EEZ 문자열을 실수로 검증·변환한다. */
bool parse_float(const char *text, float *value)
{
    if (text == nullptr || value == nullptr || text[0] == '\0') {
        return false;
    }

    errno = 0;
    char *end = nullptr;
    const float parsed = std::strtof(text, &end);
    if (errno == ERANGE || end == text || end == nullptr || *end != '\0' ||
        !std::isfinite(parsed)) {
        return false;
    }
    *value = parsed;
    return true;
}

/** @brief Preserve the distinction between an unset field and a valid zero. */
bool parse_optional_float(uint32_t variable, float *value, uint32_t *mask,
                          uint32_t bit)
{
    const char *text = get_text(variable);
    if (text[0] == '\0') return true;
    if (!parse_float(text, value)) return false;
    *mask |= bit;
    return true;
}

/** @brief [RidgeSetup 두둑 카드] 화면의 두둑 수·폭·길이·고랑값을 typed Draft로 변환한다. */
esp_err_t parse_ridge_from_ui(dajilog_ridge_settings_t *out)
{
    if (out == nullptr) return ESP_ERR_INVALID_ARG;
    int32_t ridge_count = 0;
    dajilog_ridge_settings_t settings = {};
    const char *count =
        get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_COUNT_TEXT);
    if (count[0] != '\0') {
        if (!parse_int(count, &ridge_count) || ridge_count < 0) return ESP_ERR_INVALID_ARG;
        if (ridge_count > 0) {
            settings.ridge_count = static_cast<uint32_t>(ridge_count);
            settings.presence_mask |= DAJILOG_RIDGE_PRESENT_COUNT;
        }
    }
    if (!parse_optional_float(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_WIDTH_M_TEXT,
                              &settings.ridge_width_m, &settings.presence_mask,
                              DAJILOG_RIDGE_PRESENT_WIDTH) ||
        !parse_optional_float(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_ACTUAL_LENGTH_M_TEXT,
                              &settings.actual_length_m, &settings.presence_mask,
                              DAJILOG_RIDGE_PRESENT_SECTION_LENGTH) ||
        !parse_optional_float(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_INCLUDED_FURROW_WIDTH_M_TEXT,
                              &settings.included_furrow_width_m, &settings.presence_mask,
                              DAJILOG_RIDGE_PRESENT_FURROW_WIDTH)) {
        ESP_LOGW(TAG, "ridge settings contain an invalid number");
        return ESP_ERR_INVALID_ARG;
    }
    const char *mode = get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_WORK_WIDTH_MODE);
    if (std::strcmp(mode, "RIDGE_ONLY") == 0) {
        settings.work_width_mode = DAJILOG_WORK_WIDTH_RIDGE_ONLY;
    } else if (std::strcmp(mode, "RIDGE_PLUS_FURROW") == 0) {
        settings.work_width_mode = DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW;
        if (!(settings.presence_mask & DAJILOG_RIDGE_PRESENT_FURROW_WIDTH))
            return ESP_ERR_INVALID_ARG;
    } else {
        return ESP_ERR_INVALID_ARG;
    }
    settings.presence_mask |= DAJILOG_RIDGE_PRESENT_WORK_WIDTH_MODE;
    *out = settings;
    return ESP_OK;
}

/** @brief [RidgeSetup 임시저장] 현재 Bxx 상세값과 계산용 Draft를 RAM에 함께 반영한다. */
esp_err_t stage_ridge_from_ui()
{
    dajilog_ridge_settings_t settings = {};
    const esp_err_t parse_error = parse_ridge_from_ui(&settings);
    if (parse_error != ESP_OK) return parse_error;
    dajilog_ridge_profiles_snapshot_t profiles = {};
    const esp_err_t profile_error = dajilog_system_core_get_ridge_profiles(&profiles);
    if (profile_error != ESP_OK) return profile_error;
    if (profiles.selected_id == 0U) {
        return dajilog_system_core_stage_ridge_settings(&settings);
    }
    dajilog_ridge_profile_input_t input = {};
    bool found = false;
    for (uint32_t i = 0; i < profiles.count; ++i) {
        if (profiles.profiles[i].id != profiles.selected_id) continue;
        input = profiles.profiles[i].input;
        found = true;
        break;
    }
    if (!found || !(settings.presence_mask & DAJILOG_RIDGE_PRESENT_WIDTH))
        return ESP_ERR_INVALID_STATE;
    input.width_m = settings.ridge_width_m;
    input.work_width_mode = settings.work_width_mode;
    input.furrow_width_m = settings.work_width_mode == DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW
        ? settings.included_furrow_width_m : 0.0f;
    input.split_section = eez::flow::getGlobalVariable(
        FLOW_GLOBAL_VARIABLE_RIDGE_SECTION_SPLIT_SELECTED).getBoolean() ? 1U : 0U;
    if (input.split_section) {
        if (!parse_float(get_text(FLOW_GLOBAL_VARIABLE_RIDGE_NORTH_LENGTH_M_TEXT),
                         &input.north_length_m) ||
            !parse_float(get_text(FLOW_GLOBAL_VARIABLE_RIDGE_SOUTH_LENGTH_M_TEXT),
                         &input.south_length_m)) return ESP_ERR_INVALID_ARG;
        input.whole_length_m = 0.0f;
    } else {
        if (!(settings.presence_mask & DAJILOG_RIDGE_PRESENT_SECTION_LENGTH))
            return ESP_ERR_INVALID_ARG;
        input.whole_length_m = settings.actual_length_m;
        input.north_length_m = 0.0f;
        input.south_length_m = 0.0f;
    }
    const esp_err_t error = dajilog_system_core_stage_ridge_profile_settings(
        profiles.selected_id, &input, &settings);
    if (error != ESP_OK) {
        ESP_LOGW(TAG, "ridge value is outside its schema range");
    }
    return error;
}

/** @brief [RidgeSetup 계산 결과 카드] 유효 작업폭·면적을 재계산해 화면에 표시한다. */
void render_ridge_preview()
{
    dajilog_ridge_settings_t ridge = {};
    const esp_err_t error = parse_ridge_from_ui(&ridge);
    const bool split = eez::flow::getGlobalVariable(
        FLOW_GLOBAL_VARIABLE_RIDGE_SECTION_SPLIT_SELECTED).getBoolean();
    float north = 0.0f;
    float south = 0.0f;
    const bool split_valid = !split ||
        (parse_float(get_text(FLOW_GLOBAL_VARIABLE_RIDGE_NORTH_LENGTH_M_TEXT), &north) &&
         parse_float(get_text(FLOW_GLOBAL_VARIABLE_RIDGE_SOUTH_LENGTH_M_TEXT), &south));
    const dajilog_ridge_preview_t result = error == ESP_OK && split_valid
        ? dajilog_ridge_preview_calculate(&ridge, split, north, south)
        : dajilog_ridge_preview_t{};
    char value[32];
    if (result.width_valid) {
        std::snprintf(value, sizeof(value), "%.2f", result.effective_width_m);
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_RESULT_EFFECTIVE_WIDTH_TEXT, value);
    } else {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_RESULT_EFFECTIVE_WIDTH_TEXT, "--");
    }
    if (result.area_valid) {
        std::snprintf(value, sizeof(value), "%.2f", result.total_area_m2);
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_RESULT_TOTAL_AREA_TEXT, value);
    } else {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_RESULT_TOTAL_AREA_TEXT, "--");
    }
    setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_RESULT_SURFACE_PREPARATION_TEXT, "--");
    setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_RESULT_SUBSURFACE_PREPARATION_TEXT, "--");
    setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_RESULT_SURFACE_FLOW_TEXT, "--");
    setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_RESULT_SUBSURFACE_FLOW_TEXT, "--");
    setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_RESULT_STATUS_TEXT,
                 error != ESP_OK ? "입력 형식 확인 필요"
                 : !result.width_valid ? "두둑 폭/고랑 폭 입력 필요"
                 : !result.area_valid ? (split ? "북·남 길이 입력 필요" : "전체 길이 입력 필요")
                 : "선택 두둑 면적 미리보기");
}

/** @brief [RidgeSetup 두둑 선택] 선택 Bxx의 폭·길이·고랑·구간 설정을 화면에 복원한다. */
void render_ridge_profiles(bool load_fields)
{
    dajilog_ridge_profiles_snapshot_t profiles = {};
    if (dajilog_system_core_get_ridge_profiles(&profiles) != ESP_OK) return;
    char value[32];
    if (profiles.count == 0U) {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_SELECTED_ID_TEXT, "두둑 미등록 ▼");
    } else {
        std::snprintf(value, sizeof(value), "B%02lu",
                      static_cast<unsigned long>(profiles.selected_id));
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_SELECTED_ID_TEXT, value);
    }
    std::snprintf(value, sizeof(value), "%lu",
                  static_cast<unsigned long>(profiles.count));
    setting_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_COUNT_TEXT, value);
    if (!load_fields || profiles.count == 0U || profiles.selected_id == 0U) return;
    const dajilog_ridge_profile_input_t *selected = nullptr;
    for (uint32_t i = 0; i < profiles.count; ++i) {
        if (profiles.profiles[i].id == profiles.selected_id) {
            selected = &profiles.profiles[i].input;
            break;
        }
    }
    if (selected == nullptr) return;
    const dajilog_ridge_profile_input_t &input = *selected;
    std::snprintf(value, sizeof(value), "%.2f", static_cast<double>(input.width_m));
    setting_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_WIDTH_M_TEXT, value);
    std::snprintf(value, sizeof(value), "%.2f", static_cast<double>(input.furrow_width_m));
    setting_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_INCLUDED_FURROW_WIDTH_M_TEXT,
                 input.work_width_mode == DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW ? value : "");
    std::snprintf(value, sizeof(value), "%.2f", static_cast<double>(input.whole_length_m));
    setting_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_ACTUAL_LENGTH_M_TEXT,
                 input.split_section ? "" : value);
    std::snprintf(value, sizeof(value), "%.2f", static_cast<double>(input.north_length_m));
    setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_NORTH_LENGTH_M_TEXT,
                 input.split_section ? value : "");
    std::snprintf(value, sizeof(value), "%.2f", static_cast<double>(input.south_length_m));
    setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_SOUTH_LENGTH_M_TEXT,
                 input.split_section ? value : "");
    const bool furrow = input.work_width_mode == DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW;
    setting_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_WORK_WIDTH_MODE,
                 furrow ? "RIDGE_PLUS_FURROW" : "RIDGE_ONLY");
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_ONLY_SELECTED,
                                 eez::BooleanValue(!furrow));
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_FURROW_SELECTED,
                                 eez::BooleanValue(furrow));
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_RIDGE_SECTION_SPLIT_SELECTED,
                                 eez::BooleanValue(input.split_section != 0U));
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_RIDGE_SECTION_WHOLE_SELECTED,
                                 eez::BooleanValue(input.split_section == 0U));
    render_ridge_preview();
}

/** @brief [RidgeSetup 추가·변경 팝업] 자동 Bxx 번호와 현재 폭·길이를 입력칸에 채운다. */
void prepare_ridge_form()
{
    const bool visible = eez::flow::getGlobalVariable(
        FLOW_GLOBAL_VARIABLE_RIDGE_FORM_VISIBLE).getBoolean();
    if (!visible) { s_ridge_form_was_visible = false; return; }
    if (s_ridge_form_was_visible) return;
    s_ridge_form_was_visible = true;
    dajilog_ridge_profiles_snapshot_t profiles = {};
    if (dajilog_system_core_get_ridge_profiles(&profiles) != ESP_OK) return;
    const bool edit = std::strcmp(get_text(FLOW_GLOBAL_VARIABLE_RIDGE_FORM_MODE), "EDIT") == 0;
    uint32_t id = 1U;
    const dajilog_ridge_profile_input_t *selected = nullptr;
    if (edit) {
        id = profiles.selected_id;
        for (uint32_t i = 0; i < profiles.count; ++i)
            if (profiles.profiles[i].id == id) selected = &profiles.profiles[i].input;
    } else {
        for (uint32_t i = 0; i < profiles.count; ++i)
            if (profiles.profiles[i].id == id) ++id;
    }
    char value[24];
    std::snprintf(value, sizeof(value), "B%02lu", static_cast<unsigned long>(id));
    lv_label_set_text(objects.ridge_form_auto_id_value, value);
    if (selected != nullptr) {
        std::snprintf(value, sizeof(value), "%.2f", static_cast<double>(selected->width_m));
        lv_textarea_set_text(objects.ridge_form_width_input, value);
        const float length = selected->split_section
            ? selected->north_length_m + selected->south_length_m : selected->whole_length_m;
        std::snprintf(value, sizeof(value), "%.2f", static_cast<double>(length));
        lv_textarea_set_text(objects.ridge_form_length_input, value);
    } else {
        lv_textarea_set_text(objects.ridge_form_width_input, "");
        lv_textarea_set_text(objects.ridge_form_length_input, "");
    }
}

/** @brief [RidgeSetup 새 두둑 추가] 폭·실제 길이만 보내 RAM 목록에 새 Bxx를 만든다. */
void ridge_add_request()
{
    if (s_ridge_request_sequence != 0U) {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "이전 두둑 요청 처리 중");
        return;
    }
    dajilog_ridge_profile_input_t input = {};
    dajilog_ridge_profiles_snapshot_t profiles = {};
    if (dajilog_system_core_get_ridge_profiles(&profiles) != ESP_OK) return;
    uint32_t next_id = 1U;
    for (uint32_t i = 0; i < profiles.count; ++i)
        if (profiles.profiles[i].id == next_id) ++next_id;
    char expected_id[24];
    std::snprintf(expected_id, sizeof(expected_id), "B%02lu",
                  static_cast<unsigned long>(next_id));
    if (std::strcmp(lv_label_get_text(objects.ridge_form_auto_id_value), expected_id) != 0) {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "새 두둑 ID는 자동 번호를 사용하세요");
        return;
    }
    input.work_width_mode = DAJILOG_WORK_WIDTH_RIDGE_ONLY;
    const bool valid = parse_float(lv_textarea_get_text(objects.ridge_form_width_input),
                                   &input.width_m) &&
                       parse_float(lv_textarea_get_text(objects.ridge_form_length_input),
                                   &input.whole_length_m);
    if (!valid) {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "폭·길이 숫자를 확인하세요");
        return;
    }
    dajilog_ui_msg_t command;
    dajilog_ui_msg_init(&command, DAJILOG_UI_MSG_COMMAND, DAJILOG_UI_CMD_RIDGE_ADD);
    command.payload.type = DAJILOG_UI_VALUE_BYTES;
    command.payload.value.bytes.size = sizeof(input);
    static_assert(sizeof(input) <= DAJILOG_UI_MSG_BYTES_CAPACITY,
                  "ridge input exceeds UI message capacity");
    std::memcpy(command.payload.value.bytes.data, &input, sizeof(input));
    if (dajilog_ui_adaptor_submit(&command) == ESP_OK) {
        s_ridge_request_sequence = command.sequence;
        s_ridge_pending_topic = command.topic;
        s_ridge_request_deadline = esp_timer_get_time() + 5000000;
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "새 두둑 Draft 등록 중...");
    } else {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "두둑 요청 전송 실패");
    }
}

/** @brief [RidgeSetup 두둑 변경] 선택 Bxx의 폭·실제 길이 변경을 요청한다. */
void ridge_edit_request()
{
    if (s_ridge_request_sequence != 0U) return;
    dajilog_ridge_profiles_snapshot_t profiles = {};
    if (dajilog_system_core_get_ridge_profiles(&profiles) != ESP_OK ||
        profiles.selected_id == 0U) {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT, "변경할 두둑을 선택하세요");
        return;
    }
    dajilog_ridge_profile_edit_t edit = {};
    edit.id = profiles.selected_id;
    for (uint32_t i = 0; i < profiles.count; ++i) {
        if (profiles.profiles[i].id == edit.id &&
            profiles.profiles[i].input.split_section != 0U) {
            setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                         "북·남 분할 두둑은 이 화면에서 변경할 수 없습니다");
            return;
        }
    }
    char expected_id[24];
    std::snprintf(expected_id, sizeof(expected_id), "B%02lu",
                  static_cast<unsigned long>(edit.id));
    if (std::strcmp(lv_label_get_text(objects.ridge_form_auto_id_value), expected_id) != 0) {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "두둑 ID는 변경할 수 없습니다");
        return;
    }
    if (!parse_float(lv_textarea_get_text(objects.ridge_form_width_input), &edit.width_m) ||
        !parse_float(lv_textarea_get_text(objects.ridge_form_length_input), &edit.length_m)) {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT, "폭·길이 숫자를 확인하세요");
        return;
    }
    dajilog_ui_msg_t command;
    dajilog_ui_msg_init(&command, DAJILOG_UI_MSG_COMMAND, DAJILOG_UI_CMD_RIDGE_EDIT);
    command.payload.type = DAJILOG_UI_VALUE_BYTES;
    command.payload.value.bytes.size = sizeof(edit);
    static_assert(sizeof(edit) <= DAJILOG_UI_MSG_BYTES_CAPACITY, "ridge edit too large");
    std::memcpy(command.payload.value.bytes.data, &edit, sizeof(edit));
    if (dajilog_ui_adaptor_submit(&command) == ESP_OK) {
        s_ridge_request_sequence = command.sequence;
        s_ridge_pending_topic = command.topic;
        s_ridge_request_deadline = esp_timer_get_time() + 5000000;
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT, "두둑 Draft 변경 중...");
    } else {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT, "두둑 변경 요청 실패");
    }
}

/** @brief [RidgeSetup 두둑 선택] B01→B02 순환 명령을 보내며 NVS는 쓰지 않는다. */
void ridge_select_request()
{
    if (s_ridge_request_sequence != 0U) return;
    dajilog_ridge_profiles_snapshot_t profiles = {};
    if (dajilog_system_core_get_ridge_profiles(&profiles) != ESP_OK ||
        profiles.count == 0U) {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "등록된 두둑이 없습니다");
        return;
    }
    dajilog_ui_msg_t command;
    dajilog_ui_msg_init(&command, DAJILOG_UI_MSG_COMMAND, DAJILOG_UI_CMD_RIDGE_SELECT);
    command.payload.type = DAJILOG_UI_VALUE_U32;
    uint32_t selected_index = 0U;
    for (uint32_t i = 0; i < profiles.count; ++i) {
        if (profiles.profiles[i].id == profiles.selected_id) {
            selected_index = i;
            break;
        }
    }
    command.payload.value.u32 =
        profiles.profiles[(selected_index + 1U) % profiles.count].id;
    if (dajilog_ui_adaptor_submit(&command) == ESP_OK) {
        s_ridge_request_sequence = command.sequence;
        s_ridge_pending_topic = command.topic;
        s_ridge_request_deadline = esp_timer_get_time() + 5000000;
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "두둑 선택 중...");
    }
}

/** @brief [RidgeSetup 두둑 삭제] 확인된 Bxx만 RAM 목록에서 삭제 요청한다. */
void ridge_delete_request()
{
    if (s_ridge_request_sequence != 0U) {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "이전 두둑 요청 처리 중");
        return;
    }
    dajilog_ridge_profiles_snapshot_t profiles = {};
    if (dajilog_system_core_get_ridge_profiles(&profiles) != ESP_OK ||
        profiles.count == 0U || profiles.selected_id == 0U) {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "삭제할 두둑이 없습니다");
        return;
    }
    dajilog_ui_msg_t command;
    dajilog_ui_msg_init(&command, DAJILOG_UI_MSG_COMMAND,
                        DAJILOG_UI_CMD_RIDGE_DELETE);
    command.payload.type = DAJILOG_UI_VALUE_U32;
    command.payload.value.u32 = profiles.selected_id;
    if (dajilog_ui_adaptor_submit(&command) == ESP_OK) {
        s_ridge_request_sequence = command.sequence;
        s_ridge_pending_topic = command.topic;
        s_ridge_request_deadline = esp_timer_get_time() + 5000000;
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "두둑 Draft 삭제 중...");
    } else {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "두둑 삭제 요청 실패");
    }
}

/** @brief [RidgeSetup 계산 결과 카드] 입력 또는 화면 복귀 시에만 미리보기를 갱신한다. */
void refresh_ridge_preview_if_needed()
{
    const bool active = lv_screen_active() == objects.ridge_setup;
    if (!active) {
        s_ridge_screen_was_active = false;
        return;
    }
    char inputs[sizeof(s_ridge_preview_inputs)];
    std::snprintf(inputs, sizeof(inputs), "%s|%s|%s|%s|%s|%s|%d",
                  get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_WIDTH_M_TEXT),
                  get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_ACTUAL_LENGTH_M_TEXT),
                  get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_INCLUDED_FURROW_WIDTH_M_TEXT),
                  get_text(FLOW_GLOBAL_VARIABLE_RIDGE_NORTH_LENGTH_M_TEXT),
                  get_text(FLOW_GLOBAL_VARIABLE_RIDGE_SOUTH_LENGTH_M_TEXT),
                  get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_WORK_WIDTH_MODE),
                  eez::flow::getGlobalVariable(
                      FLOW_GLOBAL_VARIABLE_RIDGE_SECTION_SPLIT_SELECTED).getBoolean());
    if (!s_ridge_screen_was_active ||
        std::strcmp(inputs, s_ridge_preview_inputs) != 0) {
        std::snprintf(s_ridge_preview_inputs, sizeof(s_ridge_preview_inputs),
                      "%s", inputs);
        render_ridge_preview();
    }
    s_ridge_screen_was_active = true;
}

/** @brief [SystemSetting 처방 카드] 질소·제품·비율 입력을 처방 RAM Draft에 반영한다. */
esp_err_t stage_fertilizer_from_ui()
{
    dajilog_prescription_settings_t settings = {};
    char codes[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY] = {};
    const int surface_index = eez::flow::getGlobalVariable(
        FLOW_GLOBAL_VARIABLE_SURFACE_PRODUCT_SELECTED_INDEX).getInt();
    const int subsurface_index = eez::flow::getGlobalVariable(
        FLOW_GLOBAL_VARIABLE_SUBSURFACE_PRODUCT_SELECTED_INDEX).getInt();
    const int indexes[DAJILOG_SETTING_ZONE_COUNT] = {surface_index, subsurface_index};
    for (size_t i = 0; i < DAJILOG_SETTING_ZONE_COUNT; ++i) {
        if (indexes[i] < 0 || indexes[i] > static_cast<int>(s_product_catalog.count))
            return ESP_ERR_INVALID_ARG;
        if (indexes[i] != 0) {
            const auto &product = s_product_catalog.products[indexes[i] - 1];
            if (!product.enabled) return ESP_ERR_INVALID_ARG;
            std::snprintf(codes[i], sizeof(codes[i]), "%s", product.code);
        }
    }
    const char *mode = get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_INPUT_MODE);
    if (std::strcmp(mode, "BY_NITROGEN") == 0) {
        settings.input_mode = DAJILOG_PRESCRIPTION_BY_NITROGEN;
    } else if (std::strcmp(mode, "DIRECT_PRODUCT") == 0) {
        settings.input_mode = DAJILOG_PRESCRIPTION_BY_PRODUCT_AMOUNT;
    } else {
        return ESP_ERR_INVALID_ARG;
    }
    settings.presence_mask = DAJILOG_PRESCRIPTION_PRESENT_INPUT_MODE;
    const bool values_valid = settings.input_mode == DAJILOG_PRESCRIPTION_BY_NITROGEN
        ? parse_optional_float(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_NITROGEN_RATE_KG_N_PER_10A_TEXT,
                               &settings.nitrogen_rate_kg_n_per_10a, &settings.presence_mask,
                               DAJILOG_PRESCRIPTION_PRESENT_NITROGEN_TARGET) &&
          parse_optional_float(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SURFACE_RATIO_PCT_TEXT,
                               &settings.surface_nitrogen_ratio_pct, &settings.presence_mask,
                               DAJILOG_PRESCRIPTION_PRESENT_SURFACE_RATIO)
        : parse_optional_float(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SURFACE_PRODUCT_RATE_KG_PER_10A_TEXT,
                               &settings.surface_product_rate_kg_per_10a, &settings.presence_mask,
                               DAJILOG_PRESCRIPTION_PRESENT_SURFACE_PRODUCT) &&
          parse_optional_float(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SUBSURFACE_PRODUCT_RATE_KG_PER_10A_TEXT,
                               &settings.subsurface_product_rate_kg_per_10a, &settings.presence_mask,
                               DAJILOG_PRESCRIPTION_PRESENT_SUBSURFACE_PRODUCT);
    if (!values_valid) {
        ESP_LOGW(TAG, "fertilizer settings contain an invalid number");
        return ESP_ERR_INVALID_ARG;
    }
    if (settings.input_mode == DAJILOG_PRESCRIPTION_BY_NITROGEN) {
        if (surface_index > 0) {
            settings.surface_fertilizer_nitrogen_pct =
                s_product_catalog.products[surface_index - 1].nitrogen_percent;
            settings.presence_mask |= DAJILOG_PRESCRIPTION_PRESENT_SURFACE_N_PCT;
        }
        if (subsurface_index > 0) {
            settings.subsurface_fertilizer_nitrogen_pct =
                s_product_catalog.products[subsurface_index - 1].nitrogen_percent;
            settings.presence_mask |= DAJILOG_PRESCRIPTION_PRESENT_SUBSURFACE_N_PCT;
        }
    }
    const esp_err_t error =
        dajilog_system_core_stage_prescription_products(&settings, codes);
    if (error != ESP_OK) {
        ESP_LOGW(TAG, "fertilizer value is outside its schema range");
    }
    return error;
}

/** @brief [SettingEdit 설정 카드] 선택 S1/S2 구역의 Draft를 편집 필드에 불러온다. */
void load_setting_editor()
{
    dajilog_product_settings_snapshot_t settings = {};
    if (dajilog_system_core_get_settings_draft(&settings) != ESP_OK) return;
    const auto &zone = settings.zones[s_selected_zone];
    char value[48];
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_NAME_TEXT, zone.name);
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_PRODUCT_TEXT, zone.product);
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_LOT_TEXT, zone.lot);
    std::snprintf(value, sizeof(value), "%.1f",
                  static_cast<double>(zone.rate_kg_per_10a));
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_PRESCRIPTION_TEXT,
                 zone.rate_kg_per_10a > 0.0f ? value : "");
    std::snprintf(value, sizeof(value), "%.1f",
                  static_cast<double>(zone.reference_speed_kmh));
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_SPEED_TEXT,
                 zone.reference_speed_kmh > 0.0f ? value : "");
    std::snprintf(value, sizeof(value), "%.1f",
                  zone.first_channel_permille / 10.0);
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_DISTRIBUTION_TEXT,
                 zone.configured ? value : "");
}

/** @brief [SettingEdit 설정 카드] 편집 문자열을 검증해 S1/S2 RAM Draft에 반영한다. */
esp_err_t stage_setting_editor()
{
    const char *name = get_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_NAME_TEXT);
    const char *product = get_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_PRODUCT_TEXT);
    const char *lot = get_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_LOT_TEXT);
    const char *speed_text = get_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_SPEED_TEXT);
    const char *distribution_text = get_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_DISTRIBUTION_TEXT);
    const bool has_speed = speed_text[0] != '\0';
    const bool has_distribution = distribution_text[0] != '\0';
    float speed = 0.0f, distribution = 0.0f;
    if (std::strlen(name) >= DAJILOG_SETTING_NAME_CAPACITY ||
        std::strlen(product) >= DAJILOG_PRODUCT_ID_CAPACITY ||
        std::strlen(lot) >= DAJILOG_LOT_ID_CAPACITY ||
        (has_speed && !parse_float(speed_text, &speed)) ||
        (has_distribution && !parse_float(distribution_text, &distribution)) ||
        (has_speed && speed <= 0.0f) ||
        distribution < 0.0f || distribution > 100.0f) {
        return ESP_ERR_INVALID_ARG;
    }
    dajilog_zone_settings_t zone = {};
    std::snprintf(zone.name, sizeof(zone.name), "%s", name);
    std::snprintf(zone.product, sizeof(zone.product), "%s", product);
    std::snprintf(zone.lot, sizeof(zone.lot), "%s", lot);
    zone.reference_speed_kmh = speed;
    zone.first_channel_permille =
        static_cast<uint16_t>(std::lround(distribution * 10.0f));
    zone.configured = name[0] != '\0' && product[0] != '\0' &&
                      has_speed && has_distribution;
    return dajilog_system_core_stage_zone_settings(s_selected_zone, &zone);
}

void setting_card_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    s_selected_zone = reinterpret_cast<uintptr_t>(lv_event_get_user_data(event)) == 0
                          ? DAJILOG_SETTING_ZONE_SURFACE
                          : DAJILOG_SETTING_ZONE_SUBSURFACE;
    s_setting_status[0] = '\0';
    render_setting_views();
    load_setting_editor();
}

void setting_edit_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    load_setting_editor();
}

/** @brief [CalibrationSelect 배출량표 카드] 등록 표 목록과 선택 채널의 상세 정보를 표시한다. */
void render_calibration_selection(void)
{
    const char *selected = get_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_SELECTED_CHANNEL);
    char channel = selected[0];
    const bool surface = s_selected_zone == DAJILOG_SETTING_ZONE_SURFACE;
    if ((surface && channel != 'A' && channel != 'B') ||
        (!surface && channel != 'C' && channel != 'D')) {
        channel = surface ? 'A' : 'C';
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_SELECTED_CHANNEL,
                     surface ? "A" : "C");
    }
    s_calibration_channel = channel;

    s_calibration_candidate_count = dajilog_calibration_catalog_list(
        channel, "", "", s_calibration_candidates, DAJILOG_CALIBRATION_MAX_PROFILES);
    if (s_calibration_candidate_count > DAJILOG_CALIBRATION_MAX_PROFILES)
        s_calibration_candidate_count = DAJILOG_CALIBRATION_MAX_PROFILES;
    if (s_calibration_candidate_index >= s_calibration_candidate_count)
        s_calibration_candidate_index = 0U;
    char list_text[sizeof(s_calibration_list_text)] = {};
    size_t used = 0U;
    s_calibration_row_count = 0U;
    const size_t total_count = s_calibration_candidate_count;
    for (size_t i = 0; i < total_count; ++i) {
        const auto &profile = s_calibration_candidates[i];
        const int written = std::snprintf(
            list_text + used, sizeof(list_text) - used,
            "%s %c  %s  r%lu  %s / %s  %s\n",
            i == s_calibration_candidate_index ? ">" : " ",
            channel, profile.table_id,
            static_cast<unsigned long>(profile.revision),
            profile.product_code, profile.product_lot[0] ? profile.product_lot : "--",
            profile.approved ? "승인" : "미승인");
        if (written < 0 || static_cast<size_t>(written) >= sizeof(list_text) - used)
            break;
        used += static_cast<size_t>(written);
        s_calibration_row_profile[s_calibration_row_count++] = i;
        if (i == s_calibration_candidate_index) {
            dajilog_calibration_point_t points[DAJILOG_CALIBRATION_MAX_POINTS] = {};
            size_t point_count = 0U;
            if (dajilog_calibration_catalog_get_points(
                    channel, profile.table_id, profile.revision, points,
                    DAJILOG_CALIBRATION_MAX_POINTS, &point_count) == ESP_OK) {
                for (size_t j = 0; j < point_count; ++j) {
                    const int point_written = std::snprintf(
                        list_text + used, sizeof(list_text) - used,
                        "    %u단계  %.1f V  %lu g/min\n",
                        static_cast<unsigned>(points[j].command_step),
                        points[j].command_decivolt / 10.0,
                        static_cast<unsigned long>(points[j].discharge_g_min));
                    if (point_written < 0 ||
                        static_cast<size_t>(point_written) >= sizeof(list_text) - used)
                        break;
                    used += static_cast<size_t>(point_written);
                    s_calibration_row_profile[s_calibration_row_count++] = i;
                }
            }
        }
    }
    if (total_count == 0U)
        std::snprintf(list_text, sizeof(list_text), "등록된 배출량표가 없습니다");
    if (objects.calibration_candidate_list && objects.calibration_list_state) {
        if (objects.calibration_list_columns)
            lv_label_set_text(objects.calibration_list_columns,
                              "표 ID / 제품·Lot / Revision · 아래는 측정점");
        lv_obj_add_flag(objects.calibration_candidate_list, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scroll_dir(objects.calibration_candidate_list, LV_DIR_VER);
        lv_obj_set_pos(objects.calibration_list_state, 16, 82);
        lv_obj_set_size(objects.calibration_list_state, 440,
                        static_cast<int32_t>(s_calibration_row_count * 24U + 28U));
        lv_obj_set_style_text_align(objects.calibration_list_state,
                                    LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    }
    set_string(FLOW_GLOBAL_VARIABLE_CALIBRATION_LIST_STATE_TEXT,
               s_calibration_list_text, sizeof(s_calibration_list_text),
               STRING_ID_SETTING_BASE + 100U, list_text);
    dajilog_calibration_summary_t candidate = {};
    const size_t count = s_calibration_candidate_count;
    if (count != 0U) candidate = s_calibration_candidates[s_calibration_candidate_index];

    dajilog_product_settings_snapshot_t draft = {};
    dajilog_product_settings_snapshot_t saved = {};
    dajilog_product_settings_snapshot_t applied = {};
    const bool have_draft = dajilog_system_core_get_settings_draft(&draft) == ESP_OK;
    const bool have_saved = dajilog_system_core_get_settings_saved(&saved) == ESP_OK && saved.saved_revision != 0U;
    const bool have_applied = dajilog_system_core_get_settings_applied(&applied) == ESP_OK;
    const size_t channel_index = (size_t)(channel - 'A');
    const auto format_binding = [](const dajilog_calibration_binding_t &binding,
                                   bool available, char *buffer, size_t capacity) {
        if (!available || binding.table_id[0] == '\0')
            std::snprintf(buffer, capacity, "--");
        else
            std::snprintf(buffer, capacity, "%s · r%lu", binding.table_id,
                          static_cast<unsigned long>(binding.revision));
    };
    char identity[72];
    format_binding(draft.calibration[channel_index], have_draft,
                   identity, sizeof(identity));
    setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_DRAFT_SUMMARY_TEXT, identity);
    format_binding(saved.calibration[channel_index], have_saved,
                   identity, sizeof(identity));
    setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_SAVED_SUMMARY_TEXT, identity);
    format_binding(applied.calibration[channel_index], have_applied,
                   identity, sizeof(identity));
    setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_APPLIED_SUMMARY_TEXT, identity);
    setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_TABLE_ID,
                 count != 0U ? candidate.table_id : "--");
    char value[96];
    if (count != 0U) {
        if (objects.calibration_detail_title) {
            std::snprintf(value, sizeof(value), "선택 표 상세 · %s / %s",
                          candidate.product_code,
                          candidate.product_lot[0] ? candidate.product_lot : "--");
            lv_label_set_text(objects.calibration_detail_title, value);
        }
        std::snprintf(value, sizeof(value), "%lu",
                      static_cast<unsigned long>(candidate.revision));
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_REVISION, value);
        std::snprintf(value, sizeof(value), "%.1f–%.1f V / %lu–%lu g/min",
                      candidate.min_decivolt / 10.0, candidate.max_decivolt / 10.0,
                      static_cast<unsigned long>(candidate.min_discharge_g_min),
                      static_cast<unsigned long>(candidate.max_discharge_g_min));
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_RANGE_TEXT, value);
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_APPROVAL_TEXT,
                     candidate.approved ? "계산 검증용 승인" : "미승인");
        dajilog_product_settings_snapshot_t current = {};
        const bool has_current = dajilog_system_core_get_settings_draft(&current) == ESP_OK;
        const size_t zone_index = static_cast<size_t>(s_selected_zone);
        const char *product = has_current ? current.selected_product_codes[zone_index] : "";
        const char *lot = has_current ? current.zones[zone_index].lot : "";
        const char *reason = !has_current ? "설정 조회 실패" :
            product[0] == '\0' ? "제품 미선택" :
            std::strcmp(candidate.product_code, product) != 0 ? "제품 불일치" :
            lot[0] && std::strcmp(candidate.product_lot, lot) != 0 ? "Lot 불일치" :
            !candidate.approved ? "미승인 표" :
            "선택 가능 · Draft만 변경";
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_REASON_TEXT, reason);
    } else {
        if (objects.calibration_detail_title)
            lv_label_set_text(objects.calibration_detail_title, "선택 표 상세 · 후보 없음");
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_REVISION, "--");
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_RANGE_TEXT, "--");
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_APPROVAL_TEXT, "--");
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_REASON_TEXT,
                     "실측·검증된 CSV 등록이 필요합니다");
    }
    setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_RESULT_TEXT,
                 s_calibration_result[0] ? s_calibration_result :
                 count ? "후보 표를 조회한 뒤 채널별로 선택하세요" :
                         "후보 없음 · 제품·Lot·채널을 확인하세요");
    if (objects.calibration_select_button) {
        dajilog_product_settings_snapshot_t current = {};
        dajilog_dashboard_snapshot_t operation = {};
        const bool ready = dajilog_system_core_get_snapshot(&operation) == ESP_OK &&
                           operation.operation_state == DAJILOG_OPERATION_READY;
        const bool product_ok = dajilog_system_core_get_settings_draft(&current) == ESP_OK &&
            std::strcmp(candidate.product_code,
                        current.selected_product_codes[static_cast<size_t>(s_selected_zone)]) == 0 &&
            (!current.zones[static_cast<size_t>(s_selected_zone)].lot[0] ||
             std::strcmp(candidate.product_lot,
                         current.zones[static_cast<size_t>(s_selected_zone)].lot) == 0);
        if (count != 0U && candidate.approved && product_ok && ready)
            lv_obj_remove_state(objects.calibration_select_button, LV_STATE_DISABLED);
        else
            lv_obj_add_state(objects.calibration_select_button, LV_STATE_DISABLED);
    }
    if (objects.calibration_save_button) {
        dajilog_dashboard_snapshot_t operation = {};
        const bool ready = dajilog_system_core_get_snapshot(&operation) == ESP_OK &&
                           operation.operation_state == DAJILOG_OPERATION_READY;
        if (ready && s_setting_request_sequence == 0U)
            lv_obj_remove_state(objects.calibration_save_button, LV_STATE_DISABLED);
        else
            lv_obj_add_state(objects.calibration_save_button, LV_STATE_DISABLED);
    }
    if (objects.calibration_confirm_button)
        lv_obj_remove_state(objects.calibration_confirm_button, LV_STATE_DISABLED);
}

/** @brief Tap a row in the candidate list to inspect its stable table identity. */
void calibration_list_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED ||
        s_calibration_candidate_count == 0U) return;
    lv_indev_t *indev = lv_event_get_indev(event);
    if (indev == nullptr) return;
    lv_point_t point;
    lv_indev_get_point(indev, &point);
    lv_area_t area;
    lv_obj_get_coords(objects.calibration_candidate_list, &area);
    const int32_t row_y = point.y - area.y1 +
        lv_obj_get_scroll_y(objects.calibration_candidate_list) - 82;
    if (row_y < 0) return;
    const size_t row = static_cast<size_t>(row_y / 24);
    if (row >= s_calibration_row_count) return;
    s_calibration_candidate_index = s_calibration_row_profile[row];
    render_calibration_selection();
}

/** @brief Put the selected candidate into RAM Draft; no NVS write. */
void calibration_select_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED ||
        s_calibration_candidate_index >= s_calibration_candidate_count) return;
    const auto &table = s_calibration_candidates[s_calibration_candidate_index];
    dajilog_dashboard_snapshot_t operation = {};
    if (dajilog_system_core_get_snapshot(&operation) != ESP_OK ||
        operation.operation_state != DAJILOG_OPERATION_READY) {
        std::snprintf(s_calibration_result, sizeof(s_calibration_result),
                      "운전 중에는 표를 변경할 수 없습니다");
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_RESULT_TEXT, s_calibration_result);
        return;
    }
    dajilog_calibration_binding_t binding = {};
    std::snprintf(binding.table_id, sizeof(binding.table_id), "%s", table.table_id);
    binding.revision = table.revision;
    binding.content_hash = table.content_hash;
    const esp_err_t error = dajilog_system_core_stage_calibration(
        s_calibration_channel, &binding);
    std::snprintf(s_calibration_result, sizeof(s_calibration_result), "%s",
                  error == ESP_OK ? "Draft 변경 완료 · 저장 전 상태" :
                  error == ESP_ERR_NOT_FOUND ? "표 선택 실패 · 후보 없음" :
                  error == ESP_ERR_INVALID_STATE ? "표 선택 실패 · 제품·Lot 불일치 또는 미승인" :
                  "표 선택 실패 · 채널과 표 정보를 확인하세요");
    render_setting_views();
    render_calibration_selection();
}

/** @brief Persist all current product Draft fields and four table choices. */
void calibration_save_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    dajilog_dashboard_snapshot_t operation = {};
    if (dajilog_system_core_get_snapshot(&operation) != ESP_OK ||
        operation.operation_state != DAJILOG_OPERATION_READY) {
        std::snprintf(s_calibration_result, sizeof(s_calibration_result),
                      "운전 중에는 저장할 수 없습니다");
    } else {
        const esp_err_t error = submit_setting_command(DAJILOG_UI_CMD_SETTINGS_SAVE);
        std::snprintf(s_calibration_result, sizeof(s_calibration_result), "%s",
                      error == ESP_OK ? "NVS 저장 요청 중 · Saved는 완료 후 갱신" :
                                        "저장 명령 전달 실패");
    }
    render_calibration_selection();
}

/** @brief Refresh catalog state on demand without reading flash in LVGL. */
void calibration_refresh_event(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_PRESSED)
        render_calibration_selection();
}

/** @brief Enter the correct physical channel before Flow opens the screen. */
void calibration_open_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_SELECTED_CHANNEL,
                 s_selected_zone == DAJILOG_SETTING_ZONE_SURFACE ? "A" : "C");
    s_calibration_candidate_index = 0U;
    s_calibration_result[0] = '\0';
    s_calibration_screen_was_active = false;
    render_calibration_selection();
}

void setting_save_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    if (lv_event_get_target(event) == objects.setting_edit_save_button &&
        stage_setting_editor() != ESP_OK) {
        std::snprintf(s_setting_status, sizeof(s_setting_status),
                      "입력 오류 · 저장할 수 없습니다");
        render_setting_views();
        return;
    }
    s_setting_pending_topic = DAJILOG_UI_CMD_SETTINGS_SAVE;
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_CONFIRM_ACTION_TEXT, "설정 저장");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_CONFIRM_MESSAGE_TEXT,
                 "저장 후에도 현재 운전 설정은 변경되지 않습니다.");
}

void setting_apply_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    s_setting_pending_topic = DAJILOG_UI_CMD_SETTINGS_CONFIRM;
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_CONFIRM_ACTION_TEXT, "설정 적용");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_CONFIRM_MESSAGE_TEXT,
                 "저장된 설정과 A~D 배출량표를 검증해 운전에 적용합니다.");
}

void setting_confirm_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED ||
        s_setting_pending_topic == 0U) return;
    const uint32_t topic = s_setting_pending_topic;
    s_setting_pending_topic = 0U;
    const esp_err_t error = submit_setting_command(topic);
    if (error != ESP_OK) {
        std::snprintf(s_setting_status, sizeof(s_setting_status),
                      "명령 전달 실패 · 다시 시도하세요");
        render_setting_views();
    }
}

void setting_cancel_event(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_PRESSED)
        s_setting_pending_topic = 0U;
}

/** @brief [두둑·처방 카드 공통] 앱 진입 시 Product Draft와 선택 Bxx를 화면에 복원한다. */
esp_err_t render_current_recipe()
{
    dajilog_product_settings_snapshot_t settings = {};
    const esp_err_t error =
        dajilog_system_core_get_settings_draft(&settings);
    if (error != ESP_OK) {
        return error;
    }

    dajilog_product_catalog_t catalog = {};
    if (dajilog_system_core_get_product_catalog(&catalog) != ESP_OK)
        return ESP_ERR_INVALID_STATE;
    s_product_catalog = catalog;
    eez::Value options = eez::Value::makeArrayRef(
        static_cast<int>(catalog.count + 1U), eez::flow::defs_v3::ARRAY_TYPE_STRING,
        0x07014000U);
    if (options.getArray() == nullptr) return ESP_ERR_NO_MEM;
    options.getArray()->values[0] = eez::Value::makeStringRef(
        "제품 선택", -1, 0x07014001U);
    for (uint32_t i = 0; i < catalog.count; ++i) {
        options.getArray()->values[i + 1U] = eez::Value::makeStringRef(
            catalog.products[i].name, -1, 0x07014002U + i);
    }
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_FERTILIZER_PRODUCT_NAMES,
                                 options);
    for (size_t zone = 0; zone < DAJILOG_SETTING_ZONE_COUNT; ++zone) {
        int index = 0;
        for (uint32_t i = 0; i < catalog.count; ++i) {
            if (std::strcmp(settings.selected_product_codes[zone],
                            catalog.products[i].code) == 0) {
                index = static_cast<int>(i + 1U);
                break;
            }
        }
        eez::flow::setGlobalVariable(
            zone == DAJILOG_SETTING_ZONE_SURFACE
                ? FLOW_GLOBAL_VARIABLE_SURFACE_PRODUCT_SELECTED_INDEX
                : FLOW_GLOBAL_VARIABLE_SUBSURFACE_PRODUCT_SELECTED_INDEX,
            eez::IntegerValue(index));
        s_last_product_index[zone] = index;
    }

    set_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_COUNT_TEXT,
             s_ridge_count_text, sizeof(s_ridge_count_text),
             STRING_ID_RIDGE_COUNT, "%.0f",
             static_cast<double>(settings.ridge.ridge_count));
    if (!(settings.ridge.presence_mask & DAJILOG_RIDGE_PRESENT_COUNT))
        set_string(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_COUNT_TEXT,
                   s_ridge_count_text, sizeof(s_ridge_count_text), STRING_ID_RIDGE_COUNT, "");
    set_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_WIDTH_M_TEXT,
             s_ridge_width_text, sizeof(s_ridge_width_text),
             STRING_ID_RIDGE_WIDTH, "%.2f",
             static_cast<double>(settings.ridge.ridge_width_m));
    if (!(settings.ridge.presence_mask & DAJILOG_RIDGE_PRESENT_WIDTH))
        set_string(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_WIDTH_M_TEXT,
                   s_ridge_width_text, sizeof(s_ridge_width_text), STRING_ID_RIDGE_WIDTH, "");
    set_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_ACTUAL_LENGTH_M_TEXT,
             s_actual_length_text, sizeof(s_actual_length_text),
             STRING_ID_ACTUAL_LENGTH, "%.1f",
             static_cast<double>(settings.ridge.actual_length_m));
    if (!(settings.ridge.presence_mask & DAJILOG_RIDGE_PRESENT_SECTION_LENGTH))
        set_string(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_ACTUAL_LENGTH_M_TEXT,
                   s_actual_length_text, sizeof(s_actual_length_text), STRING_ID_ACTUAL_LENGTH, "");
    set_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_NITROGEN_RATE_KG_N_PER_10A_TEXT,
             s_nitrogen_rate_text, sizeof(s_nitrogen_rate_text),
             STRING_ID_NITROGEN_RATE, "%g",
             static_cast<double>(
                 settings.prescription.nitrogen_rate_kg_n_per_10a));
    if (!(settings.prescription.presence_mask & DAJILOG_PRESCRIPTION_PRESENT_NITROGEN_TARGET))
        set_string(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_NITROGEN_RATE_KG_N_PER_10A_TEXT,
                   s_nitrogen_rate_text, sizeof(s_nitrogen_rate_text), STRING_ID_NITROGEN_RATE, "");

    char value[32];
#define RENDER_RECIPE_FLOAT(variable, mask, field) do {                           \
        if (settings.prescription.presence_mask & (mask)) {                        \
            std::snprintf(value, sizeof(value), "%g",                           \
                          static_cast<double>(settings.prescription.field));      \
            setting_text((variable), value);                                      \
        } else setting_text((variable), "");                                     \
    } while (0)
    RENDER_RECIPE_FLOAT(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SURFACE_RATIO_PCT_TEXT,
                        DAJILOG_PRESCRIPTION_PRESENT_SURFACE_RATIO,
                        surface_nitrogen_ratio_pct);
    if (settings.prescription.presence_mask & DAJILOG_PRESCRIPTION_PRESENT_SURFACE_RATIO) {
        std::snprintf(value, sizeof(value), "%g",
                      static_cast<double>(100.0f - settings.prescription.surface_nitrogen_ratio_pct));
        setting_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SUBSURFACE_RATIO_PCT_TEXT, value);
    } else {
        setting_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SUBSURFACE_RATIO_PCT_TEXT, "");
    }
    RENDER_RECIPE_FLOAT(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SURFACE_NITROGEN_PCT_TEXT,
                        DAJILOG_PRESCRIPTION_PRESENT_SURFACE_N_PCT,
                        surface_fertilizer_nitrogen_pct);
    RENDER_RECIPE_FLOAT(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SUBSURFACE_NITROGEN_PCT_TEXT,
                        DAJILOG_PRESCRIPTION_PRESENT_SUBSURFACE_N_PCT,
                        subsurface_fertilizer_nitrogen_pct);
    RENDER_RECIPE_FLOAT(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SURFACE_PRODUCT_RATE_KG_PER_10A_TEXT,
                        DAJILOG_PRESCRIPTION_PRESENT_SURFACE_PRODUCT,
                        surface_product_rate_kg_per_10a);
    RENDER_RECIPE_FLOAT(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SUBSURFACE_PRODUCT_RATE_KG_PER_10A_TEXT,
                        DAJILOG_PRESCRIPTION_PRESENT_SUBSURFACE_PRODUCT,
                        subsurface_product_rate_kg_per_10a);
#undef RENDER_RECIPE_FLOAT
    if (settings.ridge.presence_mask & DAJILOG_RIDGE_PRESENT_FURROW_WIDTH) {
        std::snprintf(value, sizeof(value), "%g",
                      static_cast<double>(settings.ridge.included_furrow_width_m));
        setting_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_INCLUDED_FURROW_WIDTH_M_TEXT, value);
    } else {
        setting_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_INCLUDED_FURROW_WIDTH_M_TEXT, "");
    }
    const bool furrow = (settings.ridge.presence_mask & DAJILOG_RIDGE_PRESENT_WORK_WIDTH_MODE) &&
                        settings.ridge.work_width_mode == DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW;
    setting_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_WORK_WIDTH_MODE,
                 furrow ? "RIDGE_PLUS_FURROW" : "RIDGE_ONLY");
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_ONLY_SELECTED,
                                 eez::BooleanValue(!furrow));
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_FURROW_SELECTED,
                                 eez::BooleanValue(furrow));
    const bool direct = (settings.prescription.presence_mask & DAJILOG_PRESCRIPTION_PRESENT_INPUT_MODE) &&
                        settings.prescription.input_mode == DAJILOG_PRESCRIPTION_BY_PRODUCT_AMOUNT;
    setting_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_INPUT_MODE,
                 direct ? "DIRECT_PRODUCT" : "BY_NITROGEN");
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_NITROGEN_SELECTED,
                                 eez::BooleanValue(!direct));
    eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_DIRECT_SELECTED,
                                 eez::BooleanValue(direct));
    render_ridge_profiles(true);
    return ESP_OK;
}

/** @brief Show catalog nitrogen values when an EEZ dropdown selection changes. */
void refresh_selected_products()
{
    if (lv_screen_active() != objects.system_setting) return;
    const uint32_t variables[DAJILOG_SETTING_ZONE_COUNT] = {
        FLOW_GLOBAL_VARIABLE_SURFACE_PRODUCT_SELECTED_INDEX,
        FLOW_GLOBAL_VARIABLE_SUBSURFACE_PRODUCT_SELECTED_INDEX,
    };
    const uint32_t nitrogen_variables[DAJILOG_SETTING_ZONE_COUNT] = {
        FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SURFACE_NITROGEN_PCT_TEXT,
        FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SUBSURFACE_NITROGEN_PCT_TEXT,
    };
    for (size_t zone = 0; zone < DAJILOG_SETTING_ZONE_COUNT; ++zone) {
        const int index = eez::flow::getGlobalVariable(variables[zone]).getInt();
        if (index == s_last_product_index[zone]) continue;
        s_last_product_index[zone] = index;
        if (index <= 0 || index > static_cast<int>(s_product_catalog.count)) {
            setting_text(nitrogen_variables[zone], "");
            continue;
        }
        char value[24];
        std::snprintf(value, sizeof(value), "%g",
                      static_cast<double>(s_product_catalog.products[index - 1].nitrogen_percent));
        setting_text(nitrogen_variables[zone], value);
    }
}

/** @brief [RidgeSetup 임시저장 버튼] 현재 Bxx 입력을 RAM에만 보관한다. */
void ridge_temporary_save_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) {
        return;
    }

    if (stage_ridge_from_ui() == ESP_OK) {
        SETTINGS_LOGI(
            "ridge draft: ridge_count=%s, ridge_width=%s m, actual_length=%s m",
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_COUNT_TEXT),
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_WIDTH_M_TEXT),
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_ACTUAL_LENGTH_M_TEXT));
    }
}

/** @brief [SystemSetting 임시저장 버튼] 처방 입력을 RAM에만 보관한다. */
void fertilizer_temporary_save_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) {
        return;
    }

    if (stage_fertilizer_from_ui() == ESP_OK) {
        render_setting_views();
        SETTINGS_LOGI(
            "fertilizer draft: nitrogen_rate=%s kg N/10a",
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_NITROGEN_RATE_KG_N_PER_10A_TEXT));
    }
}

esp_err_t submit_settings_apply(esp_err_t stage_error)
{
    return stage_error == ESP_OK
               ? submit_command(DAJILOG_UI_CMD_SETTINGS_APPLY)
               : stage_error;
}

/** @brief [RidgeSetup 설정 저장 버튼] 현재 Bxx를 Stage하고 Bxx 목록만 NVS 저장한다. */
void ridge_save_settings_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) {
        return;
    }
    if (s_ridge_request_sequence != 0U) {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "두둑 작업 처리 중 · 완료 후 저장하세요");
        return;
    }
    const esp_err_t error = stage_ridge_from_ui();
    if (error != ESP_OK) {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "두둑 입력 오류 · 폭/길이/구간을 확인하세요");
        ESP_LOGW(TAG, "ridge settings stage failed: %s", esp_err_to_name(error));
        return;
    }
    dajilog_ui_msg_t command;
    dajilog_ui_msg_init(&command, DAJILOG_UI_MSG_COMMAND, DAJILOG_UI_CMD_RIDGE_SAVE);
    const esp_err_t submit_error = dajilog_ui_adaptor_submit(&command);
    if (submit_error == ESP_OK) {
        s_ridge_request_sequence = command.sequence;
        s_ridge_pending_topic = command.topic;
        s_ridge_request_deadline = esp_timer_get_time() + 5000000;
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "두둑 설정 NVS 저장 중...");
    } else {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "두둑 저장 요청 실패 · 다시 시도하세요");
        ESP_LOGW(TAG, "ridge save submit failed: %s", esp_err_to_name(submit_error));
    }
}

/** @brief [SystemSetting 설정 저장 버튼] 처방을 Stage한 후 Storage Task 저장을 요청한다. */
void fertilizer_save_settings_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) {
        return;
    }

    const esp_err_t error = submit_settings_apply(stage_fertilizer_from_ui());
    if (error != ESP_OK) {
        ESP_LOGW(TAG, "fertilizer settings apply failed: %s",
                 esp_err_to_name(error));
    }
}

void stop_event(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_PRESSED) {
        s_run_deadline = 0;
        s_test_held = false;
        if (lv_screen_active() == objects.run_countdown) {
            eez::flow::replacePageHook(SCREEN_ID_DASHBOARD, 0, 0, 0);
            return;
        }
        dajilog_dashboard_snapshot_t snapshot = {};
        const bool reset = lv_event_get_target(event) == objects.dashboard_stop_button &&
            dajilog_system_core_get_snapshot(&snapshot) == ESP_OK &&
            snapshot.operation_state == DAJILOG_OPERATION_READY;
        const esp_err_t error = submit_command(reset ? DAJILOG_UI_CMD_ESTIMATE_RESET
                                                     : DAJILOG_UI_CMD_STOP);
        if (error != ESP_OK) {
            ESP_LOGW(TAG, "%s submit failed: %s", reset ? "Estimate reset" : "STOP",
                     esp_err_to_name(error));
        }
    }
}

} // namespace

/** @brief [앱 시작] 각 카드의 버튼 이벤트를 바인딩하고 저장된 Draft를 화면에 복원한다. */
extern "C" bool eez_ui_binding_attach(void)
{
    if (s_attached) {
        return true;
    }

    esp_err_t error = dajilog_system_core_start();
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "system core start failed: %s", esp_err_to_name(error));
        return false;
    }
    const dajilog_ui_adaptor_config_t adaptor_config = {
        .queue_length = DAJILOG_UI_ADAPTOR_DEFAULT_QUEUE_LENGTH,
        .render = render_message,
        .render_context = nullptr,
    };
    error = dajilog_ui_adaptor_attach(&s_adaptor, &adaptor_config);
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "UI adaptor attach failed: %s", esp_err_to_name(error));
        return false;
    }
    error = render_current_recipe();
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "recipe render failed: %s", esp_err_to_name(error));
        (void)dajilog_ui_adaptor_detach(&s_adaptor);
        return false;
    }
    if (objects.ridge_temp_save == nullptr ||
        objects.ridge_save_settings == nullptr ||
        objects.fertilizer_temp_save == nullptr ||
        objects.fertilizer_save_settings == nullptr ||
        objects.dashboard_stop_button == nullptr ||
        objects.driver_1_card == nullptr ||
        objects.setting_1_card == nullptr ||
        objects.driver_2_card == nullptr ||
        objects.setting_2_card == nullptr ||
        objects.setting_detail_info_card == nullptr ||
        objects.setting_detail_save_button == nullptr ||
        objects.setting_detail_apply_button == nullptr ||
        objects.setting_edit_save_button == nullptr ||
        objects.setting_confirm_ok_button == nullptr ||
        objects.driver_detail_channel_a_selector == nullptr ||
        objects.driver_detail_channel_b_selector == nullptr ||
        objects.driver_detail_test_mode_button == nullptr ||
        objects.driver_detail_output_slider == nullptr ||
        objects.driver_detail_trim_minus_button == nullptr ||
        objects.driver_detail_trim_plus_button == nullptr ||
        objects.driver_detail_trim_reset_button == nullptr ||
        objects.driver_detail_trim_apply_button == nullptr ||
        objects.driver_detail_close_button == nullptr) {
        ESP_LOGE(TAG, "ridge or fertilizer save buttons are unavailable");
        (void)dajilog_ui_adaptor_detach(&s_adaptor);
        return false;
    }

    lv_obj_add_event_cb(objects.ridge_temp_save, ridge_temporary_save_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.ridge_save_settings, ridge_save_settings_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.fertilizer_temp_save,
                        fertilizer_temporary_save_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.fertilizer_save_settings,
                        fertilizer_save_settings_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.dashboard_stop_button, stop_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_remove_state(objects.setting_1_card, LV_STATE_DISABLED);
    lv_obj_remove_state(objects.setting_2_card, LV_STATE_DISABLED);
    lv_obj_add_event_cb(objects.setting_1_card, setting_card_event,
                        LV_EVENT_PRESSED, (void *)0U);
    lv_obj_add_event_cb(objects.setting_2_card, setting_card_event,
                        LV_EVENT_PRESSED, (void *)1U);
    lv_obj_add_event_cb(objects.setting_detail_info_card, setting_edit_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.setting_detail_save_button, setting_save_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.setting_edit_save_button, setting_save_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.setting_detail_apply_button, setting_apply_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.setting_confirm_ok_button, setting_confirm_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.setting_confirm_cancel_button, setting_cancel_event,
                        LV_EVENT_PRESSED, nullptr);
    if (objects.setting_table_open_selection_button)
        lv_obj_add_event_cb(objects.setting_table_open_selection_button,
                            calibration_open_event, LV_EVENT_PRESSED, nullptr);
    if (objects.calibration_refresh_button)
        lv_obj_add_event_cb(objects.calibration_refresh_button,
                            calibration_refresh_event, LV_EVENT_PRESSED, nullptr);
    if (objects.calibration_candidate_list) {
        lv_obj_add_flag(objects.calibration_candidate_list, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(objects.calibration_candidate_list,
                            calibration_list_event, LV_EVENT_PRESSED, nullptr);
    }
    if (objects.calibration_select_button)
        lv_obj_add_event_cb(objects.calibration_select_button,
                            calibration_select_event, LV_EVENT_PRESSED, nullptr);
    if (objects.calibration_save_button)
        lv_obj_add_event_cb(objects.calibration_save_button,
                            calibration_save_event, LV_EVENT_PRESSED, nullptr);
    bind_driver_event(objects.driver_1_card, driver_card_event, (void *)0U);
    bind_driver_event(objects.driver_2_card, driver_card_event, (void *)1U);
    bind_driver_selector_event(objects.driver_detail_channel_a_selector, detail_channel_event, (void *)0U);
    bind_driver_selector_event(objects.driver_detail_channel_b_selector, detail_channel_event, (void *)1U);
    bind_driver_event(objects.driver_detail_auto_mode_button, detail_mode_event, (void *)1U);
    bind_driver_event(objects.driver_detail_manual_mode_button, detail_mode_event, (void *)2U);
    bind_driver_event(objects.driver_detail_test_mode_button, detail_mode_event, (void *)3U);
    bind_driver_event(objects.driver_detail_output_slider, detail_slider_event);
    bind_driver_event(objects.driver_detail_trim_minus_button, detail_trim_event, (void *)-1);
    bind_driver_event(objects.driver_detail_trim_plus_button, detail_trim_event, (void *)1);
    bind_driver_event(objects.driver_detail_trim_reset_button, detail_cancel_event);
    bind_driver_event(objects.driver_detail_trim_apply_button, detail_apply_event);
    bind_driver_event(objects.driver_detail_close_button, detail_close_event);
    bind_driver_event(objects.dashboard_run_button, run_event);
    bind_driver_event(objects.run_countdown_stop_button, stop_event);
    bind_driver_event(objects.run_readiness_refresh_button, run_readiness_refresh_event);
    // Ridge actions are emitted by the generated EEZ Flow, not LVGL CLICKED.
    // Consuming one source avoids missed taps and duplicate NVS requests.
    lv_obj_add_state(objects.ridge_recipe_ridge_count_input, LV_STATE_DISABLED);
    setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                 "두둑 추가로 저장 · B번호를 눌러 선택");
    // 생성 UI에 driver_detail_reset_button이 추가되면 detail_reset_event를 연결한다.
    // 현재는 고정 좌표의 임시 LVGL 버튼을 생성하지 않는다.
    select_driver(0U);
    render_latest_dashboard();
    render_setting_views(false);
    render_calibration_selection();
    render_ridge_profiles(true);
    render_ridge_preview();
    s_attached = true;
    s_ridge_form_was_visible = false;
    ESP_LOGI(TAG, "System Core, settings and A-D voltage binding attached");
    return true;
}

/** @brief [LVGL 주기 처리] 두둑 이벤트·UI 메시지·화면 미리보기를 한 Tick에서 처리한다. */
extern "C" void eez_ui_binding_process(void)
{
    if (s_attached) {
        (void)dajilog_ui_adaptor_process(&s_adaptor, MAX_MESSAGES_PER_TICK);
        /* EEZ Flow runs first in the timer. Its detail→edit action copies
         * formatted labels (e.g. "5.0 km/h") into edit variables. Restore
         * raw Draft fields only on that transition; a numeric/text popup
         * returning to edit must keep the user's newly entered value. */
        lv_obj_t *current_screen = lv_screen_active();
        if (current_screen == objects.setting_edit &&
            s_previous_setting_screen == objects.setting_detail) {
            load_setting_editor();
        } else if (current_screen == objects.setting_detail &&
                   s_previous_setting_screen != objects.setting_detail) {
            render_setting_views();
        }
        s_previous_setting_screen = current_screen;
        prepare_ridge_form();
        if (eez::flow::getGlobalVariable(
                FLOW_GLOBAL_VARIABLE_EVT_RIDGE_ADD).getBoolean()) {
            eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_EVT_RIDGE_ADD,
                                         eez::BooleanValue(false));
            ridge_add_request();
        }
        if (eez::flow::getGlobalVariable(
                FLOW_GLOBAL_VARIABLE_EVT_RIDGE_EDIT).getBoolean()) {
            eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_EVT_RIDGE_EDIT,
                                         eez::BooleanValue(false));
            ridge_edit_request();
        }
        if (eez::flow::getGlobalVariable(
                FLOW_GLOBAL_VARIABLE_EVT_RIDGE_SELECT).getBoolean()) {
            eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_EVT_RIDGE_SELECT,
                                         eez::BooleanValue(false));
            ridge_select_request();
        }
        if (eez::flow::getGlobalVariable(
                FLOW_GLOBAL_VARIABLE_EVT_RIDGE_DELETE).getBoolean()) {
            eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_EVT_RIDGE_DELETE,
                                         eez::BooleanValue(false));
            ridge_delete_request();
        }
        if (eez::flow::getGlobalVariable(
                FLOW_GLOBAL_VARIABLE_AUTO_STOP_NOTICE_ACK_EVENT).getBoolean()) {
            eez::flow::setGlobalVariable(
                FLOW_GLOBAL_VARIABLE_AUTO_STOP_NOTICE_ACK_EVENT,
                eez::BooleanValue(false));
            dajilog_dashboard_snapshot_t stop_snapshot = {};
            const uint32_t requested_sequence = static_cast<uint32_t>(
                eez::flow::getGlobalVariable(
                    FLOW_GLOBAL_VARIABLE_AUTO_STOP_NOTICE_ACK_SEQUENCE).getInt());
            if (dajilog_system_core_get_snapshot(&stop_snapshot) == ESP_OK &&
                requested_sequence != 0U &&
                requested_sequence == stop_snapshot.auto_stop_notice_sequence) {
                /* 알림 확인은 표시만 닫는다. STOPPING/FAULT와 출력 확인은 System Task 소유다. */
                s_auto_stop_acknowledged_sequence = requested_sequence;
                s_auto_stop_presented_sequence = requested_sequence;
                eez::flow::setGlobalVariable(
                    FLOW_GLOBAL_VARIABLE_AUTO_STOP_NOTICE_VISIBLE,
                    eez::BooleanValue(false));
                if (lv_screen_active() != objects.dashboard)
                    eez::flow::replacePageHook(SCREEN_ID_DASHBOARD, 0, 0, 0);
            } else if (stop_snapshot.auto_stop_notice_sequence != 0U) {
                eez::flow::setGlobalVariable(
                    FLOW_GLOBAL_VARIABLE_AUTO_STOP_NOTICE_VISIBLE,
                    eez::BooleanValue(true));
            }
        }
        refresh_ridge_preview_if_needed();
        refresh_selected_products();
        if (s_ridge_request_sequence != 0U &&
            esp_timer_get_time() > s_ridge_request_deadline) {
            s_ridge_request_sequence = 0U;
            s_ridge_pending_topic = 0U;
            setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                         "두둑 저장 응답 지연 · 목록을 다시 확인하세요");
        }
        const bool calibration_active = lv_screen_active() == objects.calibration_select;
        if (calibration_active) {
            const char *selected = get_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_SELECTED_CHANNEL);
            if (!s_calibration_screen_was_active || selected[0] != s_calibration_channel)
                render_calibration_selection();
        }
        s_calibration_screen_was_active = calibration_active;
        const int64_t now = esp_timer_get_time();
        if (s_apply_sequence && now >= s_apply_deadline) {
            s_apply_sequence = 0;
            select_driver(s_detail_motor_index / 2U);
            lv_label_set_text(objects.driver_detail_safety_hint, "응답 대기 종료 · 실제 적용 상태를 다시 읽었습니다");
        }
        if (s_setting_request_sequence != 0U &&
            now >= s_setting_request_deadline) {
            s_setting_request_sequence = 0U;
            std::snprintf(s_setting_status, sizeof(s_setting_status),
                          "응답 대기 종료 · 저장 상태를 다시 확인하세요");
            render_setting_views();
        }
        if (s_test_held) {
            if (lv_screen_active() != objects.driver_detail ||
                !lv_obj_has_state(objects.driver_detail_trim_apply_button, LV_STATE_PRESSED)) {
                end_test();
            } else if (now - s_test_refresh_us >= 100000) {
                if (submit_voltage_test(detail_final_decivolt()) != ESP_OK) end_test();
                s_test_refresh_us = now;
            }
        }
        if (s_run_deadline != 0) {
            if (lv_screen_active() != objects.run_countdown) {
                s_run_deadline = 0;
            } else if (now >= s_run_deadline) {
                s_run_deadline = 0;
                if (!render_run_readiness()) {
                    eez::flow::replacePageHook(SCREEN_ID_RUN_READINESS, 0, 0, 0);
                } else {
                    dajilog_dashboard_snapshot_t snapshot = {};
                    bool submitted = false;
                    if (dajilog_system_core_get_snapshot(&snapshot) == ESP_OK) {
                        const bool auto_selected = snapshot.drivers[0].mode == 1 ||
                                                   snapshot.drivers[1].mode == 1;
                        submitted = submit_run_command(auto_selected ? DAJILOG_UI_CMD_START_AUTO
                                                                     : DAJILOG_UI_CMD_START_MANUAL) == ESP_OK;
                    }
                    if (!submitted) (void)render_run_readiness();
                    eez::flow::replacePageHook(submitted ? SCREEN_ID_DASHBOARD :
                                                SCREEN_ID_RUN_READINESS, 0, 0, 0);
                }
            } else {
                std::snprintf(s_countdown_text, sizeof(s_countdown_text), "%d",
                              (int)((s_run_deadline - now + 999999) / 1000000));
                set_string(FLOW_GLOBAL_VARIABLE_RUN_COUNTDOWN_TEXT, s_countdown_text,
                           sizeof(s_countdown_text), STRING_ID_MOTOR_VOLTAGE_BASE + 10, s_countdown_text);
            }
        }
    }
}

/** @brief [앱 종료] 카드 이벤트 상태와 UI Adaptor 연결을 정리한다. */
extern "C" void eez_ui_binding_detach(void)
{
    s_previous_setting_screen = nullptr;
    if (!s_attached) {
        return;
    }

    end_test();
    s_run_deadline = 0;
    s_run_request_sequence = 0U;
    s_auto_stop_presented_sequence = 0;
    s_apply_sequence = 0;
    s_setting_request_sequence = 0;
    s_ridge_request_sequence = 0;
    s_setting_pending_topic = 0;
    (void)dajilog_ui_adaptor_detach(&s_adaptor);
    // System Core and Storage Task remain active across app close/reopen.
    s_attached = false;
    s_ridge_form_was_visible = false;
    ESP_LOGI(TAG, "Dashboard binding detached");
}
