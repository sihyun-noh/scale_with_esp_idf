#include <memory>

#include "dajilog_usb_memory_update.h"
#include "esp_brookesia.hpp"
#include "esp_err.h"
#include "esp_lib_utils.h"
#include "lvgl.h"
#include "systems/phone/esp_brookesia_phone_app.hpp"

#define APP_NAME "USB Memory Update"

using namespace esp_brookesia::systems;

namespace esp_brookesia::apps {

class UsbMemoryUpdateApp : public phone::App {
public:
    static UsbMemoryUpdateApp *requestInstance()
    {
        static UsbMemoryUpdateApp instance;
        return &instance;
    }

protected:
    UsbMemoryUpdateApp(): App(APP_NAME, nullptr, true, true, true) {}
    bool init() override { return true; }

    bool run() override
    {
        esp_err_t err = dajilog_usb_memory_update_start();
        if (err != ESP_OK) {
            ESP_UTILS_LOGE("Start USB drive updater failed: %s", esp_err_to_name(err));
        }

        lv_obj_t *screen = lv_screen_active();
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x0B1118), LV_PART_MAIN);
        lv_obj_t *panel = lv_obj_create(screen);
        lv_obj_set_size(panel, lv_pct(90), lv_pct(82));
        lv_obj_center(panel);
        lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                              LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(panel, 28, LV_PART_MAIN);
        lv_obj_set_style_pad_row(panel, 20, LV_PART_MAIN);
        lv_obj_set_style_bg_color(panel, lv_color_hex(0x17212B), LV_PART_MAIN);
        lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(panel, 24, LV_PART_MAIN);

        lv_obj_t *title = lv_label_create(panel);
        lv_label_set_text(title, "USB Memory Update");
        lv_obj_set_style_text_font(title, &lv_font_montserrat_40, LV_PART_MAIN);
        lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

        lv_obj_t *guide = lv_label_create(panel);
        lv_obj_set_width(guide, lv_pct(100));
        lv_label_set_long_mode(guide, LV_LABEL_LONG_WRAP);
        lv_label_set_text(guide,
            "1. Power the board through USB-UART\n"
            "2. Insert a FAT USB drive with an OTG adapter\n"
            "3. Tap Update when the drive is ready");
        lv_obj_set_style_text_font(guide, &lv_font_montserrat_24, LV_PART_MAIN);
        lv_obj_set_style_text_line_space(guide, 8, LV_PART_MAIN);
        lv_obj_set_style_text_align(guide, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_color(guide, lv_color_hex(0xB9C8D8), LV_PART_MAIN);

        _status = lv_label_create(panel);
        lv_obj_set_width(_status, lv_pct(100));
        lv_label_set_long_mode(_status, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_font(_status, &lv_font_montserrat_28, LV_PART_MAIN);
        lv_obj_set_style_text_line_space(_status, 8, LV_PART_MAIN);
        lv_obj_set_style_text_align(_status, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_color(_status, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

        _bar = lv_bar_create(panel);
        lv_obj_set_size(_bar, lv_pct(100), 28);
        lv_bar_set_range(_bar, 0, 100);

        _update = createButton(panel, "Update", updateEvent);
        _restart = createButton(panel, "Restart", restartEvent);
        setEnabled(_update, false);
        setEnabled(_restart, false);

        _timer = lv_timer_create(timerCallback, 250, this);
        refresh();
        return true;
    }

    bool back() override
    {
        dajilog_usb_memory_update_status_t status;
        dajilog_usb_memory_update_get_status(&status);
        if (status.state == DAJILOG_USB_MEMORY_VALIDATING ||
            status.state == DAJILOG_USB_MEMORY_WRITING) {
            return false;
        }
        return notifyCoreClosed();
    }

    bool cleanResource() override
    {
        if (_timer != nullptr) lv_timer_delete(_timer);
        _timer = nullptr;
        _status = nullptr;
        _bar = nullptr;
        _update = nullptr;
        _restart = nullptr;
        _last_state = DAJILOG_USB_MEMORY_IDLE;
        _last_progress = -1;
        return true;
    }

private:
    static lv_obj_t *createButton(lv_obj_t *parent, const char *text, lv_event_cb_t cb)
    {
        lv_obj_t *button = lv_button_create(parent);
        lv_obj_set_size(button, lv_pct(100), 70);
        lv_obj_add_event_cb(button, cb, LV_EVENT_CLICKED, requestInstance());
        lv_obj_t *label = lv_label_create(button);
        lv_label_set_text(label, text);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_28, LV_PART_MAIN);
        lv_obj_center(label);
        return button;
    }

    static void setEnabled(lv_obj_t *obj, bool enabled)
    {
        if (enabled) lv_obj_remove_state(obj, LV_STATE_DISABLED);
        else lv_obj_add_state(obj, LV_STATE_DISABLED);
    }

    static void updateEvent(lv_event_t *event)
    {
        auto *self = static_cast<UsbMemoryUpdateApp *>(lv_event_get_user_data(event));
        esp_err_t err = dajilog_usb_memory_update_apply();
        if (err != ESP_OK && self->_status != nullptr) {
            lv_label_set_text_fmt(self->_status, "Update failed: %s", esp_err_to_name(err));
        }
    }

    static void restartEvent(lv_event_t *event)
    {
        (void)event;
        dajilog_usb_memory_update_reboot();
    }

    static void timerCallback(lv_timer_t *timer)
    {
        auto *self = static_cast<UsbMemoryUpdateApp *>(lv_timer_get_user_data(timer));
        if (self != nullptr) self->refresh();
    }

    void refresh()
    {
        dajilog_usb_memory_update_status_t status;
        dajilog_usb_memory_update_get_status(&status);
        if (_status == nullptr ||
            (status.state == _last_state && status.progress_percent == _last_progress)) return;
        _last_state = status.state;
        _last_progress = status.progress_percent;
        lv_label_set_text(_status, status.message);
        lv_obj_set_style_text_color(_status,
            status.state == DAJILOG_USB_MEMORY_ERROR ? lv_color_hex(0xFF6B6B)
                                                    : lv_color_hex(0xFFFFFF),
            LV_PART_MAIN);
        lv_bar_set_value(_bar, status.progress_percent, LV_ANIM_ON);
        setEnabled(_update, status.state == DAJILOG_USB_MEMORY_MOUNTED);
        setEnabled(_restart, status.state == DAJILOG_USB_MEMORY_READY_TO_REBOOT);
    }

    lv_obj_t *_status = nullptr;
    lv_obj_t *_bar = nullptr;
    lv_obj_t *_update = nullptr;
    lv_obj_t *_restart = nullptr;
    lv_timer_t *_timer = nullptr;
    dajilog_usb_memory_update_state_t _last_state = DAJILOG_USB_MEMORY_IDLE;
    int _last_progress = -1;
};

ESP_UTILS_REGISTER_PLUGIN_WITH_CONSTRUCTOR(
    systems::base::App, UsbMemoryUpdateApp, APP_NAME,
    []() {
        return std::shared_ptr<UsbMemoryUpdateApp>(
            UsbMemoryUpdateApp::requestInstance(), [](UsbMemoryUpdateApp *) {});
    }
)

} // namespace esp_brookesia::apps
