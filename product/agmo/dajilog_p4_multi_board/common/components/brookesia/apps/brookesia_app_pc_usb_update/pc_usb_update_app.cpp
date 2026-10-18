#include <memory>

#include "dajilog_pc_usb_update.h"
#include "esp_brookesia.hpp"
#include "esp_err.h"
#include "esp_lib_utils.h"
#include "lvgl.h"
#include "systems/phone/esp_brookesia_phone_app.hpp"

#define APP_NAME "PC USB Update"

using namespace esp_brookesia::systems;

namespace esp_brookesia::apps {

class PcUsbUpdateApp : public phone::App {
public:
    static PcUsbUpdateApp *requestInstance()
    {
        static PcUsbUpdateApp instance;
        return &instance;
    }

protected:
    PcUsbUpdateApp(): App(APP_NAME, nullptr, true, true, true)
    {
    }

    bool init() override
    {
        return true;
    }

    bool run() override
    {
        esp_err_t start_error = dajilog_pc_usb_update_start();
        if (start_error != ESP_OK) {
            ESP_UTILS_LOGE("Start USB updater failed: %s", esp_err_to_name(start_error));
        }

        lv_obj_t *screen = lv_screen_active();
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x0B1118), LV_PART_MAIN);

        lv_obj_t *panel = lv_obj_create(screen);
        lv_obj_set_size(panel, lv_pct(90), lv_pct(78));
        lv_obj_center(panel);
        lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(panel, 28, LV_PART_MAIN);
        lv_obj_set_style_pad_row(panel, 22, LV_PART_MAIN);
        lv_obj_set_style_bg_color(panel, lv_color_hex(0x17212B), LV_PART_MAIN);
        lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(panel, 24, LV_PART_MAIN);

        lv_obj_t *title = lv_label_create(panel);
        lv_label_set_text(title, "PC USB Update");
        lv_obj_set_style_text_font(title, &lv_font_montserrat_40, LV_PART_MAIN);
        lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

        lv_obj_t *instructions = lv_label_create(panel);
        lv_obj_set_width(instructions, lv_pct(100));
        lv_label_set_long_mode(instructions, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_align(instructions, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_font(instructions, &lv_font_montserrat_24, LV_PART_MAIN);
        lv_obj_set_style_text_line_space(instructions, 8, LV_PART_MAIN);
        lv_obj_set_style_text_color(instructions, lv_color_hex(0xB9C8D8), LV_PART_MAIN);
        lv_label_set_text(
            instructions,
            "1. Connect the USB OTG port\n"
            "2. Copy DAJILOG_UPDATE.bin\n"
            "3. Safely eject the drive\n"
            "4. Tap Update"
        );

        _status_label = lv_label_create(panel);
        lv_obj_set_width(_status_label, lv_pct(100));
        lv_label_set_long_mode(_status_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_align(_status_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_font(_status_label, &lv_font_montserrat_28, LV_PART_MAIN);
        lv_obj_set_style_text_line_space(_status_label, 8, LV_PART_MAIN);
        lv_obj_set_style_text_color(_status_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

        _progress_bar = lv_bar_create(panel);
        lv_obj_set_size(_progress_bar, lv_pct(100), 28);
        lv_bar_set_range(_progress_bar, 0, 100);
        lv_bar_set_value(_progress_bar, 0, LV_ANIM_OFF);

        _update_button = createButton(panel, "Update", &PcUsbUpdateApp::updateButtonEvent);
        _restart_button = createButton(panel, "Restart", &PcUsbUpdateApp::restartButtonEvent);
        setButtonEnabled(_update_button, false);
        setButtonEnabled(_restart_button, false);

        _status_timer = lv_timer_create(&PcUsbUpdateApp::statusTimerCallback, 250, this);
        refreshStatus();
        return true;
    }

    bool back() override
    {
        dajilog_pc_usb_update_status_t status;
        dajilog_pc_usb_update_get_status(&status);
        if (status.state == DAJILOG_PC_USB_UPDATE_VALIDATING ||
            status.state == DAJILOG_PC_USB_UPDATE_WRITING) {
            return false;
        }
        return notifyCoreClosed();
    }

    bool cleanResource() override
    {
        if (_status_timer != nullptr) {
            lv_timer_delete(_status_timer);
        }
        _status_timer = nullptr;
        _status_label = nullptr;
        _progress_bar = nullptr;
        _update_button = nullptr;
        _restart_button = nullptr;
        _last_state = DAJILOG_PC_USB_UPDATE_IDLE;
        _last_progress = -1;
        return true;
    }

private:
    static lv_obj_t *createButton(lv_obj_t *parent, const char *text, lv_event_cb_t callback)
    {
        lv_obj_t *button = lv_button_create(parent);
        lv_obj_set_size(button, lv_pct(100), 70);
        lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, requestInstance());
        lv_obj_t *label = lv_label_create(button);
        lv_label_set_text(label, text);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_28, LV_PART_MAIN);
        lv_obj_center(label);
        return button;
    }

    static void setButtonEnabled(lv_obj_t *button, bool enabled)
    {
        if (button == nullptr) {
            return;
        }
        if (enabled) {
            lv_obj_remove_state(button, LV_STATE_DISABLED);
        } else {
            lv_obj_add_state(button, LV_STATE_DISABLED);
        }
    }

    static void setButtonText(lv_obj_t *button, const char *text)
    {
        if (button == nullptr) {
            return;
        }
        lv_obj_t *label = lv_obj_get_child(button, 0);
        if (label != nullptr) {
            lv_label_set_text(label, text);
        }
    }

    static void updateButtonEvent(lv_event_t *event)
    {
        auto *self = static_cast<PcUsbUpdateApp *>(lv_event_get_user_data(event));
        dajilog_pc_usb_update_status_t status;
        dajilog_pc_usb_update_get_status(&status);
        esp_err_t err = status.state == DAJILOG_PC_USB_UPDATE_ERROR
            ? dajilog_pc_usb_update_retry()
            : dajilog_pc_usb_update_apply();
        if (err != ESP_OK && self->_status_label != nullptr) {
            lv_label_set_text_fmt(self->_status_label, "Operation failed: %s\nTap Retry", esp_err_to_name(err));
        }
    }

    static void restartButtonEvent(lv_event_t *event)
    {
        (void)event;
        dajilog_pc_usb_update_reboot();
    }

    static void statusTimerCallback(lv_timer_t *timer)
    {
        auto *self = static_cast<PcUsbUpdateApp *>(lv_timer_get_user_data(timer));
        if (self != nullptr) {
            self->refreshStatus();
        }
    }

    void refreshStatus()
    {
        if (_status_label == nullptr) {
            return;
        }

        dajilog_pc_usb_update_status_t status;
        dajilog_pc_usb_update_get_status(&status);
        if (status.state == _last_state && status.progress_percent == _last_progress) {
            return;
        }
        _last_state = status.state;
        _last_progress = status.progress_percent;

        lv_label_set_text(_status_label, status.message);
        lv_obj_set_style_text_color(
            _status_label,
            status.state == DAJILOG_PC_USB_UPDATE_ERROR ? lv_color_hex(0xFF6B6B) : lv_color_hex(0xFFFFFF),
            LV_PART_MAIN
        );
        lv_bar_set_value(_progress_bar, status.progress_percent, LV_ANIM_ON);
        const bool retry = status.state == DAJILOG_PC_USB_UPDATE_ERROR;
        setButtonText(_update_button, retry ? "Retry" : "Update");
        setButtonEnabled(_update_button, status.state == DAJILOG_PC_USB_UPDATE_FILE_READY || retry);
        setButtonEnabled(_restart_button, status.state == DAJILOG_PC_USB_UPDATE_READY_TO_REBOOT);
    }

    lv_obj_t *_status_label = nullptr;
    lv_obj_t *_progress_bar = nullptr;
    lv_obj_t *_update_button = nullptr;
    lv_obj_t *_restart_button = nullptr;
    lv_timer_t *_status_timer = nullptr;
    dajilog_pc_usb_update_state_t _last_state = DAJILOG_PC_USB_UPDATE_IDLE;
    int _last_progress = -1;
};

ESP_UTILS_REGISTER_PLUGIN_WITH_CONSTRUCTOR(
    systems::base::App,
    PcUsbUpdateApp,
    APP_NAME,
    []() {
        return std::shared_ptr<PcUsbUpdateApp>(
            PcUsbUpdateApp::requestInstance(), [](PcUsbUpdateApp *) {}
        );
    }
)

} // namespace esp_brookesia::apps
