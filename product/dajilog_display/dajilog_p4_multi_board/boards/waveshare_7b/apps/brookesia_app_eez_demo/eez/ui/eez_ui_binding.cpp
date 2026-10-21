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

#include "generated/waveshare_7b_nongjinwon/src/ui/eez-flow.h"
#include "generated/waveshare_7b_nongjinwon/src/ui/screens.h"
#include "generated/waveshare_7b_nongjinwon/src/ui/vars.h"

namespace {

constexpr uint32_t STRING_ID_RIDGE_COUNT = 0x07010001U;
constexpr uint32_t STRING_ID_RIDGE_WIDTH = 0x07010002U;
constexpr uint32_t STRING_ID_ACTUAL_LENGTH = 0x07010003U;
constexpr uint32_t STRING_ID_NITROGEN_RATE = 0x07010004U;
constexpr uint32_t STRING_ID_DASHBOARD_BASE = 0x07011000U;
constexpr uint32_t STRING_ID_MANUAL_MOTOR_BASE = 0x07012000U;
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
char s_manual_motor_selected_text[48] = {};
char s_manual_motor_rpm_text[16] = {};
char s_manual_motor_direction_text[32] = {};
char s_manual_motor_operation_text[24] = {};
bool s_manual_motor_1_running = false;
dajilog_manual_motor_action_t s_manual_motor_direction =
    DAJILOG_MANUAL_MOTOR_FORWARD;

esp_err_t render_current_recipe();

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
    std::snprintf(storage, storage_size, "%s", value != nullptr ? value : "");
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

uint8_t manual_motor_rpm()
{
    int error = 0;
    int32_t rpm = eez::flow::getGlobalVariable(
                      FLOW_GLOBAL_VARIABLE_MANUAL_MOTOR_RPM)
                      .toInt32(&error);
    if (error != 0) {
        rpm = objects.motor_manual_rpm_slider != nullptr
                  ? lv_slider_get_value(objects.motor_manual_rpm_slider)
                  : 0;
    }
    if (rpm < 0) {
        rpm = 0;
    } else if (rpm > 30) {
        rpm = 30;
    }
    return static_cast<uint8_t>(rpm);
}

void set_manual_motor_text(uint32_t variable, char *storage,
                           size_t storage_size, uint32_t string_offset,
                           const char *text)
{
    set_string(variable, storage, storage_size,
               STRING_ID_MANUAL_MOTOR_BASE + string_offset, text);
}

void reset_manual_motor_rpm_ui()
{
    eez::flow::setGlobalVariable(
        FLOW_GLOBAL_VARIABLE_MANUAL_MOTOR_RPM,
        eez::IntegerValue(0));
    set_manual_motor_text(FLOW_GLOBAL_VARIABLE_MANUAL_MOTOR_RPM_TEXT,
                          s_manual_motor_rpm_text,
                          sizeof(s_manual_motor_rpm_text), 1, "0");
    if (objects.motor_manual_rpm_slider != nullptr) {
        lv_slider_set_value(objects.motor_manual_rpm_slider, 0, LV_ANIM_ON);
    }
}

void set_manual_motor_operation_ui(bool running)
{
    set_manual_motor_text(FLOW_GLOBAL_VARIABLE_MANUAL_MOTOR_OPERATION_TEXT,
                          s_manual_motor_operation_text,
                          sizeof(s_manual_motor_operation_text), 3,
                          running ? "RUNNING" : "STOPPED");
    eez::flow::setGlobalVariable(
        FLOW_GLOBAL_VARIABLE_MANUAL_MOTOR_RUN_ACTIVE,
        eez::BooleanValue(running));
    eez::flow::setGlobalVariable(
        FLOW_GLOBAL_VARIABLE_MANUAL_MOTOR_STOP_ACTIVE,
        eez::BooleanValue(!running));
}

esp_err_t submit_manual_motor(dajilog_manual_motor_action_t action)
{
    const dajilog_manual_motor_command_t payload = {
        .motor_index = 0,
        .rpm = manual_motor_rpm(),
        .action = action,
    };
    dajilog_ui_msg_t command;
    dajilog_ui_msg_init(&command, DAJILOG_UI_MSG_COMMAND,
                        DAJILOG_UI_CMD_MOTOR_MANUAL);
    command.payload.type = DAJILOG_UI_VALUE_BYTES;
    command.payload.value.bytes.size = sizeof(payload);
    std::memcpy(command.payload.value.bytes.data, &payload, sizeof(payload));
    return dajilog_ui_adaptor_submit(&command);
}

void manual_motor_forward_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) {
        return;
    }
    s_manual_motor_direction = DAJILOG_MANUAL_MOTOR_FORWARD;
    set_manual_motor_text(FLOW_GLOBAL_VARIABLE_MANUAL_MOTOR_DIRECTION_TEXT,
                          s_manual_motor_direction_text,
                          sizeof(s_manual_motor_direction_text), 2,
                          "FORWARD");
}

void manual_motor_reverse_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) {
        return;
    }
    s_manual_motor_direction = DAJILOG_MANUAL_MOTOR_REVERSE;
    set_manual_motor_text(FLOW_GLOBAL_VARIABLE_MANUAL_MOTOR_DIRECTION_TEXT,
                          s_manual_motor_direction_text,
                          sizeof(s_manual_motor_direction_text), 2,
                          "REVERSE");
}

void manual_motor_run_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) {
        return;
    }
    if (!eez::flow::getGlobalVariable(
             FLOW_GLOBAL_VARIABLE_DRIVER_MODE_MANUAL_SELECTED).getBoolean()) {
        s_manual_motor_1_running = false;
        set_manual_motor_operation_ui(false);
        ESP_LOGW(TAG, "manual motor 1 RUN rejected: driver mode is not MANUAL");
        return;
    }
    if (manual_motor_rpm() == 0U) {
        s_manual_motor_1_running = false;
        set_manual_motor_operation_ui(false);
        ESP_LOGW(TAG, "manual motor 1 RUN rejected: target RPM is zero");
        return;
    }
    const esp_err_t error = submit_manual_motor(s_manual_motor_direction);
    s_manual_motor_1_running = error == ESP_OK;
    set_manual_motor_operation_ui(s_manual_motor_1_running);
    if (error != ESP_OK) {
        ESP_LOGW(TAG, "manual motor 1 RUN submit failed: %s",
                 esp_err_to_name(error));
    }
}

void manual_motor_stop_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) {
        return;
    }
    s_manual_motor_1_running = false;
    const esp_err_t error = submit_manual_motor(DAJILOG_MANUAL_MOTOR_STOP);
    reset_manual_motor_rpm_ui();
    set_manual_motor_operation_ui(false);
    if (error != ESP_OK) {
        ESP_LOGW(TAG, "manual motor 1 stop submit failed: %s",
                 esp_err_to_name(error));
    }
}

void manual_motor_slider_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_RELEASED) {
        return;
    }
    const uint8_t rpm = manual_motor_rpm();
    std::snprintf(s_manual_motor_rpm_text, sizeof(s_manual_motor_rpm_text),
                  "%u", static_cast<unsigned>(rpm));
    eez::flow::setGlobalVariable(
        FLOW_GLOBAL_VARIABLE_MANUAL_MOTOR_RPM_TEXT,
        eez::Value::makeStringRef(s_manual_motor_rpm_text, -1,
                                  STRING_ID_MANUAL_MOTOR_BASE + 1));
    if (s_manual_motor_1_running) {
        const esp_err_t error = submit_manual_motor(
            DAJILOG_MANUAL_MOTOR_SET_RPM);
        if (error != ESP_OK) {
            ESP_LOGW(TAG, "manual motor 1 RPM submit failed: %s",
                     esp_err_to_name(error));
        }
    }
}

void manual_motor_close_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) {
        return;
    }
    s_manual_motor_1_running = false;
    s_manual_motor_direction = DAJILOG_MANUAL_MOTOR_FORWARD;
    (void)submit_manual_motor(DAJILOG_MANUAL_MOTOR_STOP);
    reset_manual_motor_rpm_ui();
    set_manual_motor_operation_ui(false);
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
        const esp_err_t error = submit_command(DAJILOG_UI_CMD_STOP);
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
        objects.motor_manual_close_button == nullptr ||
        objects.motor_manual_rpm_slider == nullptr ||
        objects.motor_manual_forward_button == nullptr ||
        objects.motor_manual_reverse_button == nullptr ||
        objects.motor_manual_run_button == nullptr ||
        objects.motor_manual_stop_button == nullptr) {
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
    // The first driver is the only motor currently enabled for bench testing.
    lv_obj_add_state(objects.setting_1_card, LV_STATE_DISABLED);
    lv_obj_add_state(objects.driver_2_card, LV_STATE_DISABLED);
    lv_obj_add_state(objects.setting_2_card, LV_STATE_DISABLED);
    lv_obj_add_event_cb(objects.motor_manual_forward_button,
                        manual_motor_forward_event, LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.motor_manual_reverse_button,
                        manual_motor_reverse_event, LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.motor_manual_run_button,
                        manual_motor_run_event, LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.motor_manual_stop_button,
                        manual_motor_stop_event, LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.motor_manual_rpm_slider,
                        manual_motor_slider_event, LV_EVENT_RELEASED, nullptr);
    lv_obj_add_event_cb(objects.motor_manual_close_button,
                        manual_motor_close_event, LV_EVENT_PRESSED, nullptr);
    set_manual_motor_text(FLOW_GLOBAL_VARIABLE_MANUAL_MOTOR_SELECTED_TEXT,
                          s_manual_motor_selected_text,
                          sizeof(s_manual_motor_selected_text), 0,
                          "D1 · 전층 드라이버");
    render_latest_dashboard();
    s_attached = true;
    ESP_LOGI(TAG, "System Core, settings, dashboard and M1 manual binding attached");
    return true;
}

extern "C" void eez_ui_binding_process(void)
{
    if (s_attached) {
        (void)dajilog_ui_adaptor_process(&s_adaptor, MAX_MESSAGES_PER_TICK);
    }
}

extern "C" void eez_ui_binding_detach(void)
{
    if (!s_attached) {
        return;
    }

    s_manual_motor_1_running = false;
    s_manual_motor_direction = DAJILOG_MANUAL_MOTOR_FORWARD;
    (void)submit_manual_motor(DAJILOG_MANUAL_MOTOR_STOP);
    (void)dajilog_ui_adaptor_detach(&s_adaptor);
    // System Core and Storage Task remain active across app close/reopen.
    s_attached = false;
    ESP_LOGI(TAG, "Dashboard binding detached");
}
