#include "eez_ui_binding.h"

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "dajilog_settings.h"
#include "esp_log.h"

#include "generated/waveshare_7b_nongjinwon/src/ui/eez-flow.h"
#include "generated/waveshare_7b_nongjinwon/src/ui/screens.h"
#include "generated/waveshare_7b_nongjinwon/src/ui/vars.h"

namespace {

constexpr uint32_t STRING_ID_RIDGE_COUNT = 0x07010001U;
constexpr uint32_t STRING_ID_RIDGE_WIDTH = 0x07010002U;
constexpr uint32_t STRING_ID_ACTUAL_LENGTH = 0x07010003U;
constexpr uint32_t STRING_ID_NITROGEN_RATE = 0x07010004U;

const char *TAG = "EezUiBinding7B";
bool s_attached = false;
char s_ridge_count_text[DAJILOG_SETTING_STRING_CAPACITY] = {};
char s_ridge_width_text[DAJILOG_SETTING_STRING_CAPACITY] = {};
char s_actual_length_text[DAJILOG_SETTING_STRING_CAPACITY] = {};
char s_nitrogen_rate_text[DAJILOG_SETTING_STRING_CAPACITY] = {};

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

esp_err_t stage_recipe_from_ui()
{
    int32_t ridge_count = 0;
    float ridge_width = 0.0f;
    float actual_length = 0.0f;
    float nitrogen_rate = 0.0f;

    if (!parse_int(get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_COUNT_TEXT),
                   &ridge_count) ||
        !parse_float(get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_WIDTH_M_TEXT),
                     &ridge_width) ||
        !parse_float(get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_ACTUAL_LENGTH_M_TEXT),
                     &actual_length) ||
        !parse_float(
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_NITROGEN_RATE_KG_N_PER_10A_TEXT),
            &nitrogen_rate)) {
        ESP_LOGW(TAG, "recipe contains an invalid number");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t error = dajilog_settings_set_draft_int(
        DAJILOG_SETTING_RECIPE_RIDGE_COUNT, ridge_count);
    if (error == ESP_OK) {
        error = dajilog_settings_set_draft_float(
            DAJILOG_SETTING_RECIPE_RIDGE_WIDTH_M, ridge_width);
    }
    if (error == ESP_OK) {
        error = dajilog_settings_set_draft_float(
            DAJILOG_SETTING_RECIPE_ACTUAL_LENGTH_M, actual_length);
    }
    if (error == ESP_OK) {
        error = dajilog_settings_set_draft_float(
            DAJILOG_SETTING_RECIPE_NITROGEN_RATE_KG_N_PER_10A,
            nitrogen_rate);
    }
    if (error != ESP_OK) {
        (void)dajilog_settings_discard_all_drafts();
        ESP_LOGW(TAG, "recipe value is outside its schema range");
    }
    return error;
}

esp_err_t render_current_recipe()
{
    int32_t ridge_count = 0;
    float ridge_width = 0.0f;
    float actual_length = 0.0f;
    float nitrogen_rate = 0.0f;

    esp_err_t error = dajilog_settings_get_int(
        DAJILOG_SETTING_RECIPE_RIDGE_COUNT, &ridge_count);
    if (error == ESP_OK) {
        error = dajilog_settings_get_float(
            DAJILOG_SETTING_RECIPE_RIDGE_WIDTH_M, &ridge_width);
    }
    if (error == ESP_OK) {
        error = dajilog_settings_get_float(
            DAJILOG_SETTING_RECIPE_ACTUAL_LENGTH_M, &actual_length);
    }
    if (error == ESP_OK) {
        error = dajilog_settings_get_float(
            DAJILOG_SETTING_RECIPE_NITROGEN_RATE_KG_N_PER_10A,
            &nitrogen_rate);
    }
    if (error != ESP_OK) {
        return error;
    }

    set_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_COUNT_TEXT,
             s_ridge_count_text, sizeof(s_ridge_count_text),
             STRING_ID_RIDGE_COUNT, "%.0f", static_cast<double>(ridge_count));
    set_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_WIDTH_M_TEXT,
             s_ridge_width_text, sizeof(s_ridge_width_text),
             STRING_ID_RIDGE_WIDTH, "%.2f", static_cast<double>(ridge_width));
    set_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_ACTUAL_LENGTH_M_TEXT,
             s_actual_length_text, sizeof(s_actual_length_text),
             STRING_ID_ACTUAL_LENGTH, "%.1f", static_cast<double>(actual_length));
    set_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_NITROGEN_RATE_KG_N_PER_10A_TEXT,
             s_nitrogen_rate_text, sizeof(s_nitrogen_rate_text),
             STRING_ID_NITROGEN_RATE, "%g", static_cast<double>(nitrogen_rate));
    return ESP_OK;
}

void temporary_save_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) {
        return;
    }

    if (stage_recipe_from_ui() == ESP_OK) {
        SETTINGS_LOGI(
            "draft: ridge_count=%s, ridge_width=%s m, actual_length=%s m, "
            "nitrogen_rate=%s kg N/10a",
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_COUNT_TEXT),
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_WIDTH_M_TEXT),
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_ACTUAL_LENGTH_M_TEXT),
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_NITROGEN_RATE_KG_N_PER_10A_TEXT));
    }
}

void save_settings_event(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_PRESSED) {
        return;
    }

    esp_err_t error = stage_recipe_from_ui();
    if (error == ESP_OK) {
        error = dajilog_settings_save_all();
    }
    if (error == ESP_OK) {
        SETTINGS_LOGI(
            "saved: ridge_count=%s, ridge_width=%s m, actual_length=%s m, "
            "nitrogen_rate=%s kg N/10a",
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_COUNT_TEXT),
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_RIDGE_WIDTH_M_TEXT),
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_ACTUAL_LENGTH_M_TEXT),
            get_text(FLOW_GLOBAL_VARIABLE_RECIPE_DRAFT_NITROGEN_RATE_KG_N_PER_10A_TEXT));
    } else {
        ESP_LOGW(TAG, "recipe settings apply failed: %s",
                 esp_err_to_name(error));
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
    error = render_current_recipe();
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "recipe render failed: %s", esp_err_to_name(error));
        return false;
    }
    if (objects.main_temp_save == nullptr || objects.main_save_settings == nullptr) {
        ESP_LOGE(TAG, "recipe save buttons are unavailable");
        return false;
    }

    lv_obj_add_event_cb(objects.main_temp_save, temporary_save_event,
                        LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(objects.main_save_settings, save_settings_event,
                        LV_EVENT_PRESSED, nullptr);
    s_attached = true;
    SETTINGS_LOGI("recipe settings binding attached");
    return true;
}

extern "C" void eez_ui_binding_process(void)
{
}

extern "C" void eez_ui_binding_detach(void)
{
    if (!s_attached) {
        return;
    }

    // Brookesia deletes the generated screens and their LVGL callbacks.
    s_attached = false;
    SETTINGS_LOGI("recipe settings binding detached");
}
