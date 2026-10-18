#include "eez_ui_binding.h"

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstdlib>

#include "dajilog_settings.h"
#include "dajilog_ui_adaptor.h"
#include "dajilog_ui_bridge.h"
#include "dajilog_ui_msg.h"
#include "esp_log.h"

#include "generated/brookesia_dajilog/src/ui/eez-flow.h"
#include "generated/brookesia_dajilog/src/ui/screens.h"
#include "generated/brookesia_dajilog/src/ui/vars.h"

namespace {

constexpr uint32_t TOPIC_MANUAL_TARGET = 0x05010001U;
constexpr size_t MAX_MESSAGES_PER_TICK = 4U;
constexpr uint32_t EEZ_STRING_ID = 0x05010001U;
const char *TAG = "EezUiBinding";

#ifdef CONFIG_DAJILOG_SETTINGS_DEBUG_LOG
#define SETTINGS_LOGI(format, ...) ESP_LOGI(TAG, format, ##__VA_ARGS__)
#else
#define SETTINGS_LOGI(format, ...) do { } while (0)
#endif

dajilog_ui_adaptor_t s_adaptor = {};
bool s_bridge_owned = false;
bool s_attached = false;
bool s_submit_requested = false;
char s_last_ui_value[DAJILOG_UI_MSG_TEXT_CAPACITY] = {};
char s_render_value[DAJILOG_UI_MSG_TEXT_CAPACITY] = {};

void copy_text(char *destination, size_t destination_size, const char *source)
{
    if (destination == nullptr || destination_size == 0) {
        return;
    }
    std::snprintf(destination, destination_size, "%s", source != nullptr ? source : "");
}

esp_err_t demo_command_handler(const dajilog_ui_msg_t *command, void *)
{
    if (command == nullptr || command->topic != TOPIC_MANUAL_TARGET ||
        command->payload.type != DAJILOG_UI_VALUE_TEXT) {
        return ESP_ERR_INVALID_ARG;
    }

    errno = 0;
    char *end = nullptr;
    const float requested = std::strtof(command->payload.value.text, &end);
    if (errno == ERANGE || end == command->payload.value.text ||
        end == nullptr || *end != '\0') {
        ESP_LOGW(TAG, "manual_target is not a valid number: %s",
                 command->payload.value.text);
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t error = dajilog_settings_set_draft_float(
        DAJILOG_SETTING_MANUAL_TARGET, requested);
    if (error != ESP_OK) {
        ESP_LOGW(TAG, "manual_target is outside the allowed range: %s",
                 command->payload.value.text);
        return error;
    }
    error = dajilog_settings_save_all();
    if (error != ESP_OK) {
        return error;
    }

    float applied = 0.0f;
    error = dajilog_settings_get_float(DAJILOG_SETTING_MANUAL_TARGET, &applied);
    if (error != ESP_OK) {
        return error;
    }

    // CAN/RS485 서비스 연결 전에는 설정 테이블에 적용된 값을 상태로 되돌린다.
    dajilog_ui_msg_t state;
    dajilog_ui_msg_init(&state, DAJILOG_UI_MSG_STATE, TOPIC_MANUAL_TARGET);
    state.sequence = command->sequence;
    state.payload.type = DAJILOG_UI_VALUE_TEXT;
    std::snprintf(state.payload.value.text, sizeof(state.payload.value.text),
                  "%g", static_cast<double>(applied));
    SETTINGS_LOGI("manual_target saved: %s", state.payload.value.text);
    return dajilog_ui_bridge_publish(&state);
}

void render_message(const dajilog_ui_msg_t *message, void *)
{
    if (message == nullptr || message->topic != TOPIC_MANUAL_TARGET ||
        message->kind != DAJILOG_UI_MSG_STATE ||
        message->payload.type != DAJILOG_UI_VALUE_TEXT) {
        return;
    }

    copy_text(s_render_value, sizeof(s_render_value), message->payload.value.text);
    copy_text(s_last_ui_value, sizeof(s_last_ui_value), s_render_value);
    eez::flow::setGlobalVariable(
        FLOW_GLOBAL_VARIABLE_MANUAL_TARGET_TEXT,
        eez::Value::makeStringRef(s_render_value, -1, EEZ_STRING_ID));
}

const char *get_manual_target_text()
{
    eez::Value value = eez::flow::getGlobalVariable(
        FLOW_GLOBAL_VARIABLE_MANUAL_TARGET_TEXT);
    const char *text = value.getString();
    return text != nullptr ? text : "";
}

void manual_target_done_event(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_PRESSED) {
        s_submit_requested = true;
    }
}

} // namespace

extern "C" bool eez_ui_binding_attach(void)
{
    if (s_attached) {
        return true;
    }

    esp_err_t error = dajilog_settings_init();
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "settings init failed: %s", esp_err_to_name(error));
        return false;
    }

    const dajilog_ui_bridge_config_t bridge_config = {
        .command_handler = demo_command_handler,
        .command_context = nullptr,
    };
    error = dajilog_ui_bridge_init(&bridge_config);
    if (error == ESP_OK) {
        s_bridge_owned = true;
    } else if (error != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "bridge init failed: %s", esp_err_to_name(error));
        return false;
    }

    const dajilog_ui_adaptor_config_t adaptor_config = {
        .queue_length = DAJILOG_UI_ADAPTOR_DEFAULT_QUEUE_LENGTH,
        .render = render_message,
        .render_context = nullptr,
    };
    error = dajilog_ui_adaptor_attach(&s_adaptor, &adaptor_config);
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "adaptor attach failed: %s", esp_err_to_name(error));
        if (s_bridge_owned) {
            (void)dajilog_ui_bridge_deinit();
            s_bridge_owned = false;
        }
        return false;
    }

    float manual_target = 0.0f;
    error = dajilog_settings_get_float(
        DAJILOG_SETTING_MANUAL_TARGET, &manual_target);
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "manual_target read failed: %s", esp_err_to_name(error));
        (void)dajilog_ui_adaptor_detach(&s_adaptor);
        if (s_bridge_owned) {
            (void)dajilog_ui_bridge_deinit();
            s_bridge_owned = false;
        }
        return false;
    }
    std::snprintf(s_render_value, sizeof(s_render_value), "%g",
                  static_cast<double>(manual_target));
    copy_text(s_last_ui_value, sizeof(s_last_ui_value), s_render_value);
    eez::flow::setGlobalVariable(
        FLOW_GLOBAL_VARIABLE_MANUAL_TARGET_TEXT,
        eez::Value::makeStringRef(s_render_value, -1, EEZ_STRING_ID));
    if (objects.manual_target_done_button == nullptr) {
        ESP_LOGE(TAG, "manual_target_done_button is unavailable");
        (void)dajilog_ui_adaptor_detach(&s_adaptor);
        if (s_bridge_owned) {
            (void)dajilog_ui_bridge_deinit();
            s_bridge_owned = false;
        }
        return false;
    }
    lv_obj_add_event_cb(objects.manual_target_done_button,
                        manual_target_done_event, LV_EVENT_PRESSED, nullptr);
    s_attached = true;
    SETTINGS_LOGI("manual_target_text binding attached");
    return true;
}

extern "C" void eez_ui_binding_process(void)
{
    if (!s_attached) {
        return;
    }

    (void)dajilog_ui_adaptor_process(&s_adaptor, MAX_MESSAGES_PER_TICK);
    if (!s_submit_requested) {
        return;
    }

    const char *current = get_manual_target_text();
    if (std::strncmp(current, s_last_ui_value, sizeof(s_last_ui_value)) == 0) {
        s_submit_requested = false;
        return;
    }

    dajilog_ui_msg_t command;
    dajilog_ui_msg_init(&command, DAJILOG_UI_MSG_COMMAND, TOPIC_MANUAL_TARGET);
    command.payload.type = DAJILOG_UI_VALUE_TEXT;
    copy_text(command.payload.value.text, sizeof(command.payload.value.text), current);

    const esp_err_t error = dajilog_ui_adaptor_submit(&command);
    if (error == ESP_OK) {
        copy_text(s_last_ui_value, sizeof(s_last_ui_value), current);
        s_submit_requested = false;
    } else {
        ESP_LOGW(TAG, "manual_target submit failed: %s", esp_err_to_name(error));
    }
}

extern "C" void eez_ui_binding_detach(void)
{
    if (!s_attached) {
        return;
    }

    (void)dajilog_ui_adaptor_detach(&s_adaptor);
    s_attached = false;
    s_submit_requested = false;
    std::memset(s_last_ui_value, 0, sizeof(s_last_ui_value));
    std::memset(s_render_value, 0, sizeof(s_render_value));

    if (s_bridge_owned) {
        (void)dajilog_ui_bridge_deinit();
        s_bridge_owned = false;
    }

    SETTINGS_LOGI("manual_target_text binding detached");
}
