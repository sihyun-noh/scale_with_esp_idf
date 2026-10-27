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
char s_draft_mode_text[24] = {};
char s_countdown_text[16] = {};
char s_setting_text[FLOW_GLOBAL_VARIABLE_RIDGE_RESULT_STATUS_TEXT + 1U][96] = {};
bool s_ridge_screen_was_active = false;
char s_ridge_preview_inputs[256] = {};
uint32_t s_ridge_request_sequence = 0U;
uint32_t s_ridge_pending_topic = 0U;
int64_t s_ridge_request_deadline = 0;
char s_calibration_channel = '\0';
bool s_calibration_screen_was_active = false;
dajilog_setting_zone_id_t s_selected_zone = DAJILOG_SETTING_ZONE_SURFACE;
uint32_t s_setting_request_sequence = 0U;
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

void render_dashboard(const dajilog_dashboard_snapshot_t &snapshot)
{
    set_string(FLOW_GLOBAL_VARIABLE_OPERATION_STATE_TEXT,
               s_operation_text, sizeof(s_operation_text),
               STRING_ID_DASHBOARD_BASE + 0, operation_text(snapshot.operation_state));
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
    if (snapshot.vehicle.online) {
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
        const auto &motor = snapshot.motors[i];
        set_string(driver_state_vars[i], s_motor_state_text[i], sizeof(s_motor_state_text[i]),
                   STRING_ID_DASHBOARD_BASE + 10 + i * 4,
                   motor_state_text(motor.state));
        std::snprintf(s_motor_speed_text[i], sizeof(s_motor_speed_text[i]), "%lu",
                      (unsigned long)motor.actual_rpm);
        std::snprintf(s_motor_target_text[i], sizeof(s_motor_target_text[i]), "%.1f",
                      static_cast<double>(motor.target_kg_min));
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
        const unsigned target = driver.mode == 2
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
    render_detail_voltage();
}

/** @brief Present independent setting zones without inventing calibration output. */
void render_setting_views(bool include_detail)
{
    dajilog_product_settings_snapshot_t draft = {};
    dajilog_product_settings_snapshot_t applied = {};
    if (dajilog_system_core_get_settings_draft(&draft) != ESP_OK) return;
    const bool has_applied =
        dajilog_system_core_get_settings_applied(&applied) == ESP_OK;
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
        setting_text(target_vars[i], "--");
        setting_text(table_vars[i], "승인 배출량표 없음");
        setting_text(i == 0 ? FLOW_GLOBAL_VARIABLE_SETTING_1_STATE_TEXT :
                              FLOW_GLOBAL_VARIABLE_SETTING_2_STATE_TEXT, state);
        setting_text(i == 0 ? FLOW_GLOBAL_VARIABLE_SETTING_1_NAME_TEXT :
                              FLOW_GLOBAL_VARIABLE_SETTING_2_NAME_TEXT,
                     zone.name[0] ? zone.name : "미설정");
        setting_text(i == 0 ? FLOW_GLOBAL_VARIABLE_SETTING_1_TARGET_KG_MIN_TEXT :
                              FLOW_GLOBAL_VARIABLE_SETTING_2_TARGET_KG_MIN_TEXT, "--");
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
                 zone.configured ? value : "--");
    std::snprintf(value, sizeof(value), "%.1f km/h",
                  static_cast<double>(zone.reference_speed_kmh));
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_SPEED_TEXT,
                 zone.configured ? value : "--");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_TOTAL_TARGET_TEXT, "--");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_ORIGINAL_TOTAL_TEXT, "--");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_REFERENCE_TOTAL_TEXT, "--");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_CALIBRATION_TEXT,
                 "승인 배출량표 없음");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_TABLE_STATUS_TEXT,
                 "배출량표 미연결 · 적용 차단");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_VALIDATION_TEXT,
                 "승인 배출량표가 필요합니다");
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_APPLY_BLOCK_TEXT,
                 "배출량표 확인 전 적용할 수 없습니다");
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
    setting_text(FLOW_GLOBAL_VARIABLE_SETTING_DETAIL_FOOTER_STATUS_TEXT,
                 s_setting_status[0] ? s_setting_status :
                 editable ? "설정 입력과 저장 가능 · 적용은 배출량표 필요" :
                            "운전 정지 후 변경할 수 있습니다");
    if (objects.setting_detail_apply_button &&
        !lv_obj_has_state(objects.setting_detail_apply_button, LV_STATE_DISABLED))
        lv_obj_add_state(objects.setting_detail_apply_button, LV_STATE_DISABLED);
    if (objects.setting_edit_apply_button &&
        !lv_obj_has_state(objects.setting_edit_apply_button, LV_STATE_DISABLED))
        lv_obj_add_state(objects.setting_edit_apply_button, LV_STATE_DISABLED);
    if (objects.setting_detail_edit_button &&
        lv_obj_has_state(objects.setting_detail_edit_button, LV_STATE_DISABLED) == editable)
        (editable ? lv_obj_remove_state : lv_obj_add_state)(
            objects.setting_detail_edit_button, LV_STATE_DISABLED);
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
        objects.setting_edit_draft_button,
        objects.setting_edit_save_button,
    };
    for (lv_obj_t *control : edit_controls) {
        if (control && lv_obj_has_state(control, LV_STATE_DISABLED) == editable)
            (editable ? lv_obj_remove_state : lv_obj_add_state)(
                control, LV_STATE_DISABLED);
    }
}

void render_latest_dashboard()
{
    dajilog_dashboard_snapshot_t snapshot = {};
    if (dajilog_system_core_get_snapshot(&snapshot) == ESP_OK) {
        render_dashboard(snapshot);
    }
}

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
                         s_ridge_pending_topic == DAJILOG_UI_CMD_RIDGE_DELETE
                             ? "두둑 삭제 완료" : "두둑 목록 NVS 저장 완료");
        } else {
            setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                         s_ridge_pending_topic == DAJILOG_UI_CMD_RIDGE_DELETE
                             ? "두둑 삭제 실패 · 기존 목록 유지" :
                               "두둑 등록/선택 실패 · 입력 또는 저장소 확인");
        }
        s_ridge_pending_topic = 0U;
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
        std::snprintf(s_setting_status, sizeof(s_setting_status), "%s",
                      message->result == DAJILOG_UI_RESULT_OK
                          ? "설정 저장 완료 · 현재 운전에는 미적용"
                          : "설정 처리 실패 · 입력과 저장소를 확인하세요");
        (void)render_current_recipe();
        render_setting_views();
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

/** @brief Submit one setting command and correlate its asynchronous result. */
esp_err_t submit_setting_command(uint32_t topic)
{
    dajilog_ui_msg_t command;
    dajilog_ui_msg_init(&command, DAJILOG_UI_MSG_COMMAND, topic);
    const esp_err_t error = dajilog_ui_adaptor_submit(&command);
    if (error == ESP_OK) {
        s_setting_request_sequence = command.sequence;
        s_setting_request_deadline = esp_timer_get_time() + 5000000;
        std::snprintf(s_setting_status, sizeof(s_setting_status),
                      "설정 처리 중...");
        render_setting_views();
    }
    return error;
}

/** @brief Maintain the driver editor draft and submit requests without direct hardware access. */
uint16_t detail_final_decivolt()
{
    const int32_t value =
        (int32_t)s_detail_output_step * DAJILOG_MOTOR_DECIVOLT_PER_STEP +
        s_detail_trim_decivolt;
    return static_cast<uint16_t>(value < 0 ? 0 :
        (value > DAJILOG_MOTOR_OUTPUT_DECIVOLT_MAX
             ? static_cast<int32_t>(DAJILOG_MOTOR_OUTPUT_DECIVOLT_MAX) : value));
}

/** @brief Maintain the driver editor draft and submit requests without direct hardware access. */
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
                                 eez::BooleanValue(s_driver_draft.mode == 2 || s_detail_service_test));
}

/** @brief Maintain the driver editor draft and submit requests without direct hardware access. */
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
        ? "자동 계산 미구현 · 적용할 수 없습니다"
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
void detail_reset_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    end_test();
    select_driver(s_detail_motor_index / 2U);
}

/** @brief Start a cancellable three-second countdown before the manual RUN request. */
void run_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    dajilog_dashboard_snapshot_t snapshot = {};
    if (dajilog_system_core_get_snapshot(&snapshot) != ESP_OK ||
        snapshot.operation_state != DAJILOG_OPERATION_READY || s_test_held) return;
    s_run_deadline = esp_timer_get_time() + 3000000;
    eez::flow::replacePageHook(SCREEN_ID_RUN_COUNTDOWN, 0, 0, 0);
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

/** @brief Convert visible EEZ fields to typed Draft input without I/O. */
esp_err_t parse_ridge_from_ui(dajilog_ridge_settings_t *out)
{
    if (out == nullptr) return ESP_ERR_INVALID_ARG;
    int32_t ridge_count = 0;
    dajilog_ridge_settings_t settings = {};
    const char *count =
        get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_COUNT_TEXT);
    if (count[0] != '\0') {
        if (!parse_int(count, &ridge_count) || ridge_count <= 0) return ESP_ERR_INVALID_ARG;
        settings.ridge_count = static_cast<uint32_t>(ridge_count);
        settings.presence_mask |= DAJILOG_RIDGE_PRESENT_COUNT;
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

esp_err_t stage_ridge_from_ui()
{
    dajilog_ridge_settings_t settings = {};
    const esp_err_t parse_error = parse_ridge_from_ui(&settings);
    if (parse_error != ESP_OK) return parse_error;
    const esp_err_t error = dajilog_system_core_stage_ridge_settings(&settings);
    if (error != ESP_OK) {
        ESP_LOGW(TAG, "ridge value is outside its schema range");
    }
    return error;
}

/** @brief Render only Backend draft width/area; other results stay unavailable. */
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

/** @brief Render the committed Bxx list summary and optionally its selected fields. */
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

/** @brief Submit validated UI input; only Storage Task commits NVS. */
void ridge_add_request()
{
    if (s_ridge_request_sequence != 0U) {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "이전 두둑 요청 처리 중");
        return;
    }
    dajilog_ridge_profile_input_t input = {};
    const char *mode = get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_WORK_WIDTH_MODE);
    if (std::strcmp(mode, "RIDGE_ONLY") == 0)
        input.work_width_mode = DAJILOG_WORK_WIDTH_RIDGE_ONLY;
    else if (std::strcmp(mode, "RIDGE_PLUS_FURROW") == 0)
        input.work_width_mode = DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW;
    else {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "작업폭 방식을 선택하세요");
        return;
    }
    input.split_section = eez::flow::getGlobalVariable(
        FLOW_GLOBAL_VARIABLE_RIDGE_SECTION_SPLIT_SELECTED).getBoolean() ? 1U : 0U;
    bool valid = parse_float(get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_WIDTH_M_TEXT),
                             &input.width_m);
    if (input.work_width_mode == DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW)
        valid = valid && parse_float(
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_INCLUDED_FURROW_WIDTH_M_TEXT),
            &input.furrow_width_m);
    if (input.split_section)
        valid = valid && parse_float(
            get_text(FLOW_GLOBAL_VARIABLE_RIDGE_NORTH_LENGTH_M_TEXT), &input.north_length_m) &&
            parse_float(get_text(FLOW_GLOBAL_VARIABLE_RIDGE_SOUTH_LENGTH_M_TEXT),
                        &input.south_length_m);
    else
        valid = valid && parse_float(
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_ACTUAL_LENGTH_M_TEXT),
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
                     "두둑 NVS 저장 중...");
    } else {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "두둑 요청 전송 실패");
    }
}

/** @brief Cycle through saved Bxx profiles when EEZ raises the select action. */
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
                     "두둑 선택 저장 중...");
    }
}

/** @brief Delete only the confirmed selected Bxx through System/Storage Task. */
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
                     "두둑 삭제 저장 중...");
    } else {
        setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                     "두둑 삭제 요청 실패");
    }
}

/** @brief Recompute only when source fields change or RidgeSetup reopens. */
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

esp_err_t stage_fertilizer_from_ui()
{
    dajilog_prescription_settings_t settings = {};
    const char *mode = get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_INPUT_MODE);
    if (std::strcmp(mode, "BY_NITROGEN") == 0) {
        settings.input_mode = DAJILOG_PRESCRIPTION_BY_NITROGEN;
    } else if (std::strcmp(mode, "DIRECT_PRODUCT") == 0) {
        settings.input_mode = DAJILOG_PRESCRIPTION_BY_PRODUCT_AMOUNT;
    } else {
        return ESP_ERR_INVALID_ARG;
    }
    settings.presence_mask = DAJILOG_PRESCRIPTION_PRESENT_INPUT_MODE;
    if (!parse_optional_float(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_NITROGEN_RATE_KG_N_PER_10A_TEXT,
                              &settings.nitrogen_rate_kg_n_per_10a, &settings.presence_mask,
                              DAJILOG_PRESCRIPTION_PRESENT_NITROGEN_TARGET) ||
        !parse_optional_float(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SURFACE_RATIO_PCT_TEXT,
                              &settings.surface_nitrogen_ratio_pct, &settings.presence_mask,
                              DAJILOG_PRESCRIPTION_PRESENT_SURFACE_RATIO) ||
        !parse_optional_float(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SURFACE_NITROGEN_PCT_TEXT,
                              &settings.surface_fertilizer_nitrogen_pct, &settings.presence_mask,
                              DAJILOG_PRESCRIPTION_PRESENT_SURFACE_N_PCT) ||
        !parse_optional_float(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SUBSURFACE_NITROGEN_PCT_TEXT,
                              &settings.subsurface_fertilizer_nitrogen_pct, &settings.presence_mask,
                              DAJILOG_PRESCRIPTION_PRESENT_SUBSURFACE_N_PCT) ||
        !parse_optional_float(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SURFACE_PRODUCT_RATE_KG_PER_10A_TEXT,
                              &settings.surface_product_rate_kg_per_10a, &settings.presence_mask,
                              DAJILOG_PRESCRIPTION_PRESENT_SURFACE_PRODUCT) ||
        !parse_optional_float(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_SUBSURFACE_PRODUCT_RATE_KG_PER_10A_TEXT,
                              &settings.subsurface_product_rate_kg_per_10a, &settings.presence_mask,
                              DAJILOG_PRESCRIPTION_PRESENT_SUBSURFACE_PRODUCT)) {
        ESP_LOGW(TAG, "fertilizer settings contain an invalid number");
        return ESP_ERR_INVALID_ARG;
    }
    const esp_err_t error =
        dajilog_system_core_stage_prescription_settings(&settings);
    if (error != ESP_OK) {
        ESP_LOGW(TAG, "fertilizer value is outside its schema range");
    }
    return error;
}

/** @brief Load the selected zone draft into EEZ's six edit fields. */
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

/** @brief Convert EEZ strings to a typed, RAM-only zone draft. */
esp_err_t stage_setting_editor()
{
    const char *name = get_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_NAME_TEXT);
    const char *product = get_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_PRODUCT_TEXT);
    const char *lot = get_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_LOT_TEXT);
    const char *rate_text = get_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_PRESCRIPTION_TEXT);
    const char *speed_text = get_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_SPEED_TEXT);
    const char *distribution_text = get_text(FLOW_GLOBAL_VARIABLE_SETTING_EDIT_DISTRIBUTION_TEXT);
    const bool has_rate = rate_text[0] != '\0';
    const bool has_speed = speed_text[0] != '\0';
    const bool has_distribution = distribution_text[0] != '\0';
    float rate = 0.0f, speed = 0.0f, distribution = 0.0f;
    if (std::strlen(name) >= DAJILOG_SETTING_NAME_CAPACITY ||
        std::strlen(product) >= DAJILOG_PRODUCT_ID_CAPACITY ||
        std::strlen(lot) >= DAJILOG_LOT_ID_CAPACITY ||
        (has_rate && !parse_float(rate_text, &rate)) ||
        (has_speed && !parse_float(speed_text, &speed)) ||
        (has_distribution && !parse_float(distribution_text, &distribution)) ||
        (has_rate && rate <= 0.0f) || (has_speed && speed <= 0.0f) ||
        distribution < 0.0f || distribution > 100.0f) {
        return ESP_ERR_INVALID_ARG;
    }
    dajilog_zone_settings_t zone = {};
    std::snprintf(zone.name, sizeof(zone.name), "%s", name);
    std::snprintf(zone.product, sizeof(zone.product), "%s", product);
    std::snprintf(zone.lot, sizeof(zone.lot), "%s", lot);
    zone.rate_kg_per_10a = rate;
    zone.reference_speed_kmh = speed;
    zone.first_channel_permille =
        static_cast<uint16_t>(std::lround(distribution * 10.0f));
    zone.configured = name[0] != '\0' && product[0] != '\0' &&
                      has_rate && has_speed && has_distribution;
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

/** @brief Show only catalog-backed candidates; an empty catalog never implies approval. */
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

    dajilog_product_settings_snapshot_t settings = {};
    const bool have_settings =
        dajilog_system_core_get_settings_draft(&settings) == ESP_OK;
    const dajilog_zone_settings_t *zone =
        have_settings ? &settings.zones[s_selected_zone] : nullptr;
    dajilog_calibration_summary_t candidate = {};
    size_t count = dajilog_calibration_catalog_list(
        channel, zone != nullptr ? zone->product : "",
        zone != nullptr ? zone->lot : "", &candidate, 1U);
#ifdef CONFIG_DAJILOG_CALIBRATION_SAMPLE_TEST
    if (count == 0U) {
        count = dajilog_calibration_catalog_list(
            channel, "DEMO_ONLY", "SYNTHETIC", &candidate, 1U);
    }
#endif

    setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_DRAFT_SUMMARY_TEXT,
                 "선택 안 됨");
    setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_SAVED_SUMMARY_TEXT,
                 "선택 안 됨");
    setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_APPLIED_SUMMARY_TEXT,
                 "선택 안 됨");
    setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_TABLE_ID,
                 count != 0U ? candidate.table_id : "--");
    char value[96];
    if (count != 0U) {
        std::snprintf(value, sizeof(value), "%lu",
                      static_cast<unsigned long>(candidate.revision));
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_REVISION, value);
        std::snprintf(value, sizeof(value), "%.1f~%.1f V · %lu~%lu g/min",
                      candidate.min_decivolt / 10.0, candidate.max_decivolt / 10.0,
                      static_cast<unsigned long>(candidate.min_discharge_g_min),
                      static_cast<unsigned long>(candidate.max_discharge_g_min));
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_RANGE_TEXT, value);
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_APPROVAL_TEXT,
                     candidate.approved ? "승인" : "가상 데이터 · 미승인");
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_LIST_STATE_TEXT,
                     "가상 테스트 후보 · 실제 제품과 불일치");
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_REASON_TEXT,
                     "화면 조회 시험만 가능 · 운전 적용 금지");
    } else {
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_REVISION, "--");
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_RANGE_TEXT, "--");
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_APPROVAL_TEXT, "--");
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_LIST_STATE_TEXT,
                     "등록된 배출량표 후보 없음");
        setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_CANDIDATE_REASON_TEXT,
                     "실측·검증된 CSV 등록이 필요합니다");
    }
    setting_text(FLOW_GLOBAL_VARIABLE_CALIBRATION_RESULT_TEXT,
                 count != 0U
                     ? "가상 데이터입니다. 모터 자동 제어는 차단됩니다"
                     : "표가 없어 선택·저장·작업 전 확인이 차단됩니다");
    if (objects.calibration_select_button)
        lv_obj_add_state(objects.calibration_select_button, LV_STATE_DISABLED);
    if (objects.calibration_save_button)
        lv_obj_add_state(objects.calibration_save_button, LV_STATE_DISABLED);
    if (objects.calibration_confirm_button)
        lv_obj_add_state(objects.calibration_confirm_button, LV_STATE_DISABLED);
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
    s_calibration_screen_was_active = false;
    render_calibration_selection();
}

void setting_draft_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    const esp_err_t error = stage_setting_editor();
    std::snprintf(s_setting_status, sizeof(s_setting_status), "%s",
                  error == ESP_OK ? "임시 저장 완료 · 운전에는 미반영" :
                  "입력 오류 · 설정명/제품/처방량/속도/배분을 확인하세요");
    render_setting_views();
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
                 "승인 배출량표가 없어 적용할 수 없습니다.");
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

esp_err_t render_current_recipe()
{
    dajilog_product_settings_snapshot_t settings = {};
    const esp_err_t error =
        dajilog_system_core_get_settings_draft(&settings);
    if (error != ESP_OK) {
        return error;
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

void fertilizer_temporary_save_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) {
        return;
    }

    if (stage_fertilizer_from_ui() == ESP_OK) {
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

void ridge_save_settings_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) {
        return;
    }

    const esp_err_t error = submit_settings_apply(stage_ridge_from_ui());
    if (error != ESP_OK) {
        ESP_LOGW(TAG, "ridge settings apply failed: %s",
                 esp_err_to_name(error));
    }
}

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
        const esp_err_t error = submit_command(DAJILOG_UI_CMD_STOP);
        if (lv_screen_active() == objects.run_countdown)
            eez::flow::replacePageHook(SCREEN_ID_DASHBOARD, 0, 0, 0);
        if (error != ESP_OK) {
            ESP_LOGW(TAG, "STOP submit failed: %s", esp_err_to_name(error));
        }
    }
}

} // namespace

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
        objects.setting_detail_edit_button == nullptr ||
        objects.setting_detail_save_button == nullptr ||
        objects.setting_detail_apply_button == nullptr ||
        objects.setting_edit_draft_button == nullptr ||
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
    lv_obj_add_event_cb(objects.setting_detail_edit_button, setting_edit_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.setting_detail_draft_button, setting_draft_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.setting_edit_draft_button, setting_draft_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.setting_detail_save_button, setting_save_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.setting_edit_save_button, setting_save_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.setting_detail_apply_button, setting_apply_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.setting_edit_apply_button, setting_apply_event,
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
    // Ridge actions are emitted by the generated EEZ Flow, not LVGL CLICKED.
    // Consuming one source avoids missed taps and duplicate NVS requests.
    lv_obj_add_state(objects.ridge_recipe_ridge_count_input, LV_STATE_DISABLED);
    setting_text(FLOW_GLOBAL_VARIABLE_RIDGE_REGISTRATION_STATUS_TEXT,
                 "두둑 추가로 저장 · B번호를 눌러 선택");
    // Generator has no reset widget yet. Add one without editing generated sources.
    lv_obj_t *reset = lv_button_create(lv_obj_get_parent(objects.driver_detail_trim_reset_button));
    lv_obj_set_pos(reset, 212, 478);
    lv_obj_set_size(reset, 130, 32);
    lv_obj_t *reset_label = lv_label_create(reset);
    lv_obj_set_style_text_font(reset_label, lv_obj_get_style_text_font(objects.driver_detail_trim_reset_label, LV_PART_MAIN), 0);
    lv_label_set_text(reset_label, "초기화");
    lv_obj_center(reset_label);
    bind_driver_event(reset, detail_reset_event);
    select_driver(0U);
    render_latest_dashboard();
    render_setting_views(false);
    render_calibration_selection();
    render_ridge_profiles(true);
    render_ridge_preview();
    s_attached = true;
    ESP_LOGI(TAG, "System Core, settings and A-D voltage binding attached");
    return true;
}

extern "C" void eez_ui_binding_process(void)
{
    if (s_attached) {
        (void)dajilog_ui_adaptor_process(&s_adaptor, MAX_MESSAGES_PER_TICK);
        if (eez::flow::getGlobalVariable(
                FLOW_GLOBAL_VARIABLE_EVT_RIDGE_ADD).getBoolean()) {
            eez::flow::setGlobalVariable(FLOW_GLOBAL_VARIABLE_EVT_RIDGE_ADD,
                                         eez::BooleanValue(false));
            ridge_add_request();
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
        refresh_ridge_preview_if_needed();
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
                (void)submit_command(DAJILOG_UI_CMD_START_MANUAL);
                eez::flow::replacePageHook(SCREEN_ID_DASHBOARD, 0, 0, 0);
            } else {
                std::snprintf(s_countdown_text, sizeof(s_countdown_text), "%d",
                              (int)((s_run_deadline - now + 999999) / 1000000));
                set_string(FLOW_GLOBAL_VARIABLE_RUN_COUNTDOWN_TEXT, s_countdown_text,
                           sizeof(s_countdown_text), STRING_ID_MOTOR_VOLTAGE_BASE + 10, s_countdown_text);
            }
        }
    }
}

extern "C" void eez_ui_binding_detach(void)
{
    if (!s_attached) {
        return;
    }

    end_test();
    s_run_deadline = 0;
    s_apply_sequence = 0;
    s_setting_request_sequence = 0;
    s_ridge_request_sequence = 0;
    s_setting_pending_topic = 0;
    (void)dajilog_ui_adaptor_detach(&s_adaptor);
    // System Core and Storage Task remain active across app close/reopen.
    s_attached = false;
    ESP_LOGI(TAG, "Dashboard binding detached");
}
