#include "eez_ui_binding.h"

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "dajilog_system_core.h"
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
void end_test();
void select_driver(size_t driver);
void detail_close_event(lv_event_t *event);

esp_err_t render_current_recipe();
void render_detail_voltage();

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
               message->result == DAJILOG_UI_RESULT_OK) {
        (void)render_current_recipe();
    }
}

esp_err_t submit_command(uint32_t topic)
{
    dajilog_ui_msg_t command;
    dajilog_ui_msg_init(&command, DAJILOG_UI_MSG_COMMAND, topic);
    return dajilog_ui_adaptor_submit(&command);
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

esp_err_t stage_ridge_from_ui()
{
    int32_t ridge_count = 0;
    float ridge_width = 0.0f;
    float actual_length = 0.0f;

    if (!parse_int(get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_COUNT_TEXT),
                   &ridge_count) ||
        !parse_float(get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_WIDTH_M_TEXT),
                     &ridge_width) ||
        !parse_float(get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_ACTUAL_LENGTH_M_TEXT),
                     &actual_length)) {
        ESP_LOGW(TAG, "ridge settings contain an invalid number");
        return ESP_ERR_INVALID_ARG;
    }

    const dajilog_ridge_settings_t settings = {
        .presence_mask = DAJILOG_RIDGE_PRESENT_COUNT |
                         DAJILOG_RIDGE_PRESENT_WIDTH |
                         DAJILOG_RIDGE_PRESENT_SECTION_LENGTH,
        .ridge_count = static_cast<uint32_t>(ridge_count),
        .ridge_width_m = ridge_width,
        .actual_length_m = actual_length,
    };
    const esp_err_t error =
        ridge_count > 0
            ? dajilog_system_core_stage_ridge_settings(&settings)
            : ESP_ERR_INVALID_ARG;
    if (error != ESP_OK) {
        ESP_LOGW(TAG, "ridge value is outside its schema range");
    }
    return error;
}

esp_err_t stage_fertilizer_from_ui()
{
    float nitrogen_rate = 0.0f;
    if (!parse_float(
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_NITROGEN_RATE_KG_N_PER_10A_TEXT),
            &nitrogen_rate)) {
        ESP_LOGW(TAG, "fertilizer settings contain an invalid number");
        return ESP_ERR_INVALID_ARG;
    }

    const dajilog_prescription_settings_t settings = {
        .presence_mask = DAJILOG_PRESCRIPTION_PRESENT_INPUT_MODE |
                         DAJILOG_PRESCRIPTION_PRESENT_NITROGEN_TARGET,
        .input_mode = DAJILOG_PRESCRIPTION_BY_NITROGEN,
        .nitrogen_rate_kg_n_per_10a = nitrogen_rate,
    };
    const esp_err_t error =
        dajilog_system_core_stage_prescription_settings(&settings);
    if (error != ESP_OK) {
        ESP_LOGW(TAG, "fertilizer value is outside its schema range");
    }
    return error;
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
    set_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_WIDTH_M_TEXT,
             s_ridge_width_text, sizeof(s_ridge_width_text),
             STRING_ID_RIDGE_WIDTH, "%.2f",
             static_cast<double>(settings.ridge.ridge_width_m));
    set_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_ACTUAL_LENGTH_M_TEXT,
             s_actual_length_text, sizeof(s_actual_length_text),
             STRING_ID_ACTUAL_LENGTH, "%.1f",
             static_cast<double>(settings.ridge.actual_length_m));
    set_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_NITROGEN_RATE_KG_N_PER_10A_TEXT,
             s_nitrogen_rate_text, sizeof(s_nitrogen_rate_text),
             STRING_ID_NITROGEN_RATE, "%g",
             static_cast<double>(
                 settings.prescription.nitrogen_rate_kg_n_per_10a));
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
    lv_obj_add_state(objects.setting_1_card, LV_STATE_DISABLED);
    lv_obj_add_state(objects.setting_2_card, LV_STATE_DISABLED);
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
    s_attached = true;
    ESP_LOGI(TAG, "System Core, settings and A-D voltage binding attached");
    return true;
}

extern "C" void eez_ui_binding_process(void)
{
    if (s_attached) {
        (void)dajilog_ui_adaptor_process(&s_adaptor, MAX_MESSAGES_PER_TICK);
        const int64_t now = esp_timer_get_time();
        if (s_apply_sequence && now >= s_apply_deadline) {
            s_apply_sequence = 0;
            select_driver(s_detail_motor_index / 2U);
            lv_label_set_text(objects.driver_detail_safety_hint, "응답 대기 종료 · 실제 적용 상태를 다시 읽었습니다");
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
    (void)dajilog_ui_adaptor_detach(&s_adaptor);
    // System Core and Storage Task remain active across app close/reopen.
    s_attached = false;
    ESP_LOGI(TAG, "Dashboard binding detached");
}
