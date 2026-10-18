#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "lvgl.h"
#include "esp_brookesia.hpp"
#include "esp_lib_utils.h"
#include "systems/phone/esp_brookesia_phone_app.hpp"

#define APP_NAME "Wi-Fi"

using namespace esp_brookesia::systems;

namespace esp_brookesia::apps {

class WifiApp : public phone::App {
public:
    static WifiApp *requestInstance()
    {
        static WifiApp instance;
        return &instance;
    }

protected:
    WifiApp(): App(APP_NAME, nullptr, true, true, true)
    {
    }

    bool init() override
    {
        if (_wifi_ready) {
            return true;
        }

        esp_err_t err = nvs_flash_init();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(err));
            return false;
        }

        err = esp_netif_init();
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            ESP_LOGE(TAG, "esp_netif_init failed: %s", esp_err_to_name(err));
            return false;
        }

        err = esp_event_loop_create_default();
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            ESP_LOGE(TAG, "event loop init failed: %s", esp_err_to_name(err));
            return false;
        }

        _sta_netif = esp_netif_create_default_wifi_sta();
        if (_sta_netif == nullptr) {
            ESP_LOGE(TAG, "Failed to create Wi-Fi station netif");
            return false;
        }

        wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
        err = esp_wifi_init(&wifi_init_config);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(err));
            return false;
        }

        ESP_ERROR_CHECK(esp_event_handler_instance_register(
            WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiApp::eventHandler, this, &_wifi_event_instance
        ));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(
            IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiApp::eventHandler, this, &_ip_event_instance
        ));

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
        err = esp_wifi_start();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "esp_wifi_start failed: %s", esp_err_to_name(err));
            return false;
        }

        _wifi_ready = true;

        wifi_config_t saved_config = {};
        if (loadCredentials(saved_config)) {
            err = esp_wifi_set_config(WIFI_IF_STA, &saved_config);
            if (err == ESP_OK) {
                _connection_state.store(ConnectionState::CONNECTING);
                err = esp_wifi_connect();
            }
            if (err != ESP_OK) {
                ESP_LOGW(TAG, "Automatic Wi-Fi connection failed to start: %s", esp_err_to_name(err));
                _connection_state.store(ConnectionState::FAILED);
            } else {
                ESP_LOGI(TAG, "Automatic Wi-Fi connection started");
            }
        } else {
            _connection_state.store(ConnectionState::DISCONNECTED);
        }
        return true;
    }

    bool run() override
    {
        lv_obj_t *screen = lv_scr_act();
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x101820), LV_PART_MAIN);

        lv_obj_t *panel = lv_obj_create(screen);
        lv_obj_set_size(panel, lv_pct(92), lv_pct(80));
        lv_obj_center(panel);
        lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(panel, 20, LV_PART_MAIN);
        lv_obj_set_style_pad_row(panel, 14, LV_PART_MAIN);

        lv_obj_t *title = lv_label_create(panel);
        lv_label_set_text(title, "Wi-Fi Setup");

        _network_dropdown = lv_dropdown_create(panel);
        lv_obj_set_width(_network_dropdown, lv_pct(100));
        lv_dropdown_set_options(_network_dropdown, "Tap Scan to find networks");

        lv_obj_t *scan_button = lv_button_create(panel);
        lv_obj_set_width(scan_button, lv_pct(100));
        lv_obj_add_event_cb(scan_button, &WifiApp::scanButtonEvent, LV_EVENT_CLICKED, this);
        lv_obj_t *scan_label = lv_label_create(scan_button);
        lv_label_set_text(scan_label, "Scan");
        lv_obj_center(scan_label);

        _password_textarea = lv_textarea_create(panel);
        lv_obj_set_width(_password_textarea, lv_pct(100));
        lv_textarea_set_one_line(_password_textarea, true);
        lv_textarea_set_password_mode(_password_textarea, true);
        lv_textarea_set_placeholder_text(_password_textarea, "Wi-Fi password (blank for open network)");

        lv_obj_t *connect_button = lv_button_create(panel);
        lv_obj_set_width(connect_button, lv_pct(100));
        lv_obj_add_event_cb(connect_button, &WifiApp::connectButtonEvent, LV_EVENT_CLICKED, this);
        lv_obj_t *connect_label = lv_label_create(connect_button);
        lv_label_set_text(connect_label, "Connect");
        lv_obj_center(connect_label);

        _status_label = lv_label_create(panel);
        lv_obj_set_width(_status_label, lv_pct(100));
        lv_obj_set_style_text_align(_status_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_label_set_long_mode(_status_label, LV_LABEL_LONG_WRAP);
        lv_label_set_text(_status_label, "Ready");

        _keyboard = lv_keyboard_create(panel);
        lv_obj_set_size(_keyboard, lv_pct(100), lv_pct(42));
        lv_keyboard_set_map(
            _keyboard, LV_KEYBOARD_MODE_TEXT_LOWER, KEYBOARD_LOWER_MAP, KEYBOARD_CTRL_MAP
        );
        lv_keyboard_set_map(
            _keyboard, LV_KEYBOARD_MODE_TEXT_UPPER, KEYBOARD_UPPER_MAP, KEYBOARD_CTRL_MAP
        );
        lv_keyboard_set_map(
            _keyboard, LV_KEYBOARD_MODE_SPECIAL, KEYBOARD_SPECIAL_MAP, KEYBOARD_CTRL_MAP
        );
        lv_keyboard_set_mode(_keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
        lv_keyboard_set_popovers(_keyboard, true);
        lv_keyboard_set_textarea(_keyboard, _password_textarea);

        lv_obj_set_style_bg_color(_keyboard, lv_color_hex(0xDDE2E7), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(_keyboard, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_width(_keyboard, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(_keyboard, 18, LV_PART_MAIN);
        lv_obj_set_style_pad_all(_keyboard, 8, LV_PART_MAIN);
        lv_obj_set_style_pad_row(_keyboard, 7, LV_PART_MAIN);
        lv_obj_set_style_pad_column(_keyboard, 5, LV_PART_MAIN);

        lv_obj_set_style_bg_color(_keyboard, lv_color_hex(0xFFFFFF), LV_PART_ITEMS);
        lv_obj_set_style_bg_opa(_keyboard, LV_OPA_COVER, LV_PART_ITEMS);
        lv_obj_set_style_text_color(_keyboard, lv_color_hex(0x161A1D), LV_PART_ITEMS);
        lv_obj_set_style_radius(_keyboard, 10, LV_PART_ITEMS);
        lv_obj_set_style_border_width(_keyboard, 0, LV_PART_ITEMS);
        lv_obj_set_style_shadow_width(_keyboard, 3, LV_PART_ITEMS);
        lv_obj_set_style_shadow_opa(_keyboard, LV_OPA_20, LV_PART_ITEMS);
        lv_obj_set_style_shadow_offset_y(_keyboard, 2, LV_PART_ITEMS);
        lv_obj_set_style_bg_color(
            _keyboard, lv_color_hex(0xB8C0C8),
            static_cast<lv_style_selector_t>(LV_PART_ITEMS) | LV_STATE_CHECKED
        );
        lv_obj_set_style_bg_color(
            _keyboard, lv_color_hex(0xAEB7C0),
            static_cast<lv_style_selector_t>(LV_PART_ITEMS) | LV_STATE_PRESSED
        );

        // Make the primary Wi-Fi controls 10% taller after LVGL resolves their theme sizes.
        lv_obj_update_layout(panel);
        lv_obj_t *controls[] = {
            _network_dropdown, scan_button, _password_textarea, connect_button
        };
        for (lv_obj_t *control : controls) {
            lv_obj_set_height(control, lv_obj_get_height(control) * 110 / 100);
        }

        _status_timer = lv_timer_create(&WifiApp::statusTimerCallback, 250, this);
        scanNetworks();
        return true;
    }

    bool back() override
    {
        return notifyCoreClosed();
    }

    bool cleanResource() override
    {
        _network_dropdown = nullptr;
        _password_textarea = nullptr;
        _status_label = nullptr;
        _keyboard = nullptr;
        _status_timer = nullptr;
        _last_rendered_state = ConnectionState::UNKNOWN;
        return true;
    }

private:
    static constexpr lv_buttonmatrix_ctrl_t KEY = LV_BUTTONMATRIX_CTRL_WIDTH_1;
    static constexpr lv_buttonmatrix_ctrl_t ACTION_KEY =
        static_cast<lv_buttonmatrix_ctrl_t>(
            static_cast<uint32_t>(LV_BUTTONMATRIX_CTRL_WIDTH_2) |
            static_cast<uint32_t>(LV_BUTTONMATRIX_CTRL_CHECKED)
        );

    inline static const char *const KEYBOARD_LOWER_MAP[] = {
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",
        "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
        "a", "s", "d", "f", "g", "h", "j", "k", "l", "\n",
        "ABC", "z", "x", "c", "v", "b", "n", "m", LV_SYMBOL_BACKSPACE, "\n",
        "1#", ",", " ", ".", LV_SYMBOL_OK, ""
    };

    inline static const char *const KEYBOARD_UPPER_MAP[] = {
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",
        "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "\n",
        "A", "S", "D", "F", "G", "H", "J", "K", "L", "\n",
        "abc", "Z", "X", "C", "V", "B", "N", "M", LV_SYMBOL_BACKSPACE, "\n",
        "1#", ",", " ", ".", LV_SYMBOL_OK, ""
    };

    inline static const char *const KEYBOARD_SPECIAL_MAP[] = {
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",
        "@", "#", "$", "%", "&", "-", "+", "(", ")", "/", "\n",
        "*", "\"", "'", ":", ";", "!", "?", "_", "=", "\n",
        "abc", "[", "]", "{", "}", "<", ">", "\\", LV_SYMBOL_BACKSPACE, "\n",
        "abc", ",", " ", ".", LV_SYMBOL_OK, ""
    };

    inline static const lv_buttonmatrix_ctrl_t KEYBOARD_CTRL_MAP[] = {
        KEY, KEY, KEY, KEY, KEY, KEY, KEY, KEY, KEY, KEY,
        KEY, KEY, KEY, KEY, KEY, KEY, KEY, KEY, KEY, KEY,
        KEY, KEY, KEY, KEY, KEY, KEY, KEY, KEY, KEY,
        ACTION_KEY, KEY, KEY, KEY, KEY, KEY, KEY, KEY, ACTION_KEY,
        ACTION_KEY, KEY, LV_BUTTONMATRIX_CTRL_WIDTH_8, KEY, ACTION_KEY
    };

    enum class ConnectionState {
        UNKNOWN,
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        FAILED,
    };

    static constexpr const char *TAG = "brookesia_wifi";
    static constexpr const char *NVS_NAMESPACE = "wifi_app";
    static constexpr const char *NVS_KEY_SSID = "ssid";
    static constexpr const char *NVS_KEY_PASSWORD = "password";

    static void eventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {
        auto *self = static_cast<WifiApp *>(arg);
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
            self->_save_credentials_on_connect.store(false);
            self->_connection_state.store(ConnectionState::FAILED);
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            auto *event = static_cast<ip_event_got_ip_t *>(event_data);
            std::snprintf(
                self->_ip_address, sizeof(self->_ip_address), IPSTR, IP2STR(&event->ip_info.ip)
            );
            self->_connection_state.store(ConnectionState::CONNECTED);
            if (self->_save_credentials_on_connect.exchange(false)) {
                if (xTaskCreate(
                        &WifiApp::saveCredentialsTask, "wifi_nvs_save", 4096, self, 5, nullptr
                    ) != pdPASS) {
                    ESP_LOGW(TAG, "Failed to create Wi-Fi credential save task");
                }
            }
            self->showConnectedNotification();
        }
    }

    static void saveCredentialsTask(void *arg)
    {
        auto *self = static_cast<WifiApp *>(arg);
        self->saveCurrentCredentials();
        vTaskDelete(nullptr);
    }

    static void notificationTimerCallback(lv_timer_t *timer)
    {
        auto *self = static_cast<WifiApp *>(lv_timer_get_user_data(timer));
        if (self->_notification != nullptr && lv_obj_is_valid(self->_notification)) {
            lv_obj_delete(self->_notification);
        }
        self->_notification = nullptr;
        self->_notification_timer = nullptr;
        lv_timer_delete(timer);
    }

    static void scanButtonEvent(lv_event_t *event)
    {
        auto *self = static_cast<WifiApp *>(lv_event_get_user_data(event));
        self->scanNetworks();
    }

    static void connectButtonEvent(lv_event_t *event)
    {
        auto *self = static_cast<WifiApp *>(lv_event_get_user_data(event));
        self->connectSelectedNetwork();
    }

    static void statusTimerCallback(lv_timer_t *timer)
    {
        auto *self = static_cast<WifiApp *>(lv_timer_get_user_data(timer));
        self->updateStatusUi();
    }

    bool loadCredentials(wifi_config_t &config)
    {
        nvs_handle_t handle = 0;
        esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            return false;
        }
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "Opening Wi-Fi credentials failed: %s", esp_err_to_name(err));
            return false;
        }

        char ssid[sizeof(config.sta.ssid) + 1] = {};
        char password[sizeof(config.sta.password) + 1] = {};
        size_t ssid_size = sizeof(ssid);
        size_t password_size = sizeof(password);
        err = nvs_get_str(handle, NVS_KEY_SSID, ssid, &ssid_size);
        if (err == ESP_OK) {
            err = nvs_get_str(handle, NVS_KEY_PASSWORD, password, &password_size);
        }
        nvs_close(handle);

        if (err != ESP_OK || ssid[0] == '\0') {
            if (err != ESP_ERR_NVS_NOT_FOUND) {
                ESP_LOGW(TAG, "Reading Wi-Fi credentials failed: %s", esp_err_to_name(err));
            }
            return false;
        }

        std::memcpy(config.sta.ssid, ssid, std::min(std::strlen(ssid), sizeof(config.sta.ssid)));
        std::memcpy(
            config.sta.password, password, std::min(std::strlen(password), sizeof(config.sta.password))
        );
        config.sta.threshold.authmode = WIFI_AUTH_OPEN;
        config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
        return true;
    }

    void saveCurrentCredentials()
    {
        wifi_config_t config = {};
        esp_err_t err = esp_wifi_get_config(WIFI_IF_STA, &config);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "Reading active Wi-Fi configuration failed: %s", esp_err_to_name(err));
            return;
        }

        char ssid[sizeof(config.sta.ssid) + 1] = {};
        char password[sizeof(config.sta.password) + 1] = {};
        std::memcpy(ssid, config.sta.ssid, sizeof(config.sta.ssid));
        std::memcpy(password, config.sta.password, sizeof(config.sta.password));

        nvs_handle_t handle = 0;
        err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
        if (err == ESP_OK) {
            err = nvs_set_str(handle, NVS_KEY_SSID, ssid);
        }
        if (err == ESP_OK) {
            err = nvs_set_str(handle, NVS_KEY_PASSWORD, password);
        }
        if (err == ESP_OK) {
            err = nvs_commit(handle);
        }
        if (handle != 0) {
            nvs_close(handle);
        }

        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Wi-Fi credentials saved");
        } else {
            ESP_LOGW(TAG, "Saving Wi-Fi credentials failed: %s", esp_err_to_name(err));
        }
    }

    void showConnectedNotification()
    {
        gui::LvLockGuard gui_guard;
        getSystem()->getDisplay().getStatusBar()->setWifiIconState(phone::StatusBar::WifiState::SIGNAL_3);

        if (_notification_timer != nullptr) {
            lv_timer_delete(_notification_timer);
            _notification_timer = nullptr;
        }
        if (_notification != nullptr && lv_obj_is_valid(_notification)) {
            lv_obj_delete(_notification);
        }

        _notification = lv_obj_create(lv_layer_top());
        lv_obj_set_size(_notification, lv_pct(82), LV_SIZE_CONTENT);
        lv_obj_align(_notification, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(_notification, lv_color_hex(0x20252B), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(_notification, LV_OPA_90, LV_PART_MAIN);
        lv_obj_set_style_border_width(_notification, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(_notification, 16, LV_PART_MAIN);
        lv_obj_set_style_pad_all(_notification, 18, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(_notification, 12, LV_PART_MAIN);
        lv_obj_set_style_shadow_opa(_notification, LV_OPA_30, LV_PART_MAIN);

        lv_obj_t *label = lv_label_create(_notification);
        lv_label_set_text_fmt(label, "WiFi connected : IP %s", _ip_address);
        lv_obj_set_width(label, lv_pct(100));
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_32, LV_PART_MAIN);

        _notification_timer = lv_timer_create(&WifiApp::notificationTimerCallback, 3000, this);
    }

    void scanNetworks()
    {
        if (!_wifi_ready || _network_dropdown == nullptr || _status_label == nullptr) {
            return;
        }

        lv_label_set_text(_status_label, "Scanning...");
        wifi_scan_config_t scan_config = {};
        scan_config.show_hidden = false;

        esp_err_t err = esp_wifi_scan_start(&scan_config, true);
        if (err != ESP_OK) {
            lv_label_set_text_fmt(_status_label, "Scan failed: %s", esp_err_to_name(err));
            return;
        }

        uint16_t ap_count = 0;
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
        ap_count = std::min<uint16_t>(ap_count, 20);

        std::vector<wifi_ap_record_t> records(ap_count);
        if (ap_count > 0) {
            ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, records.data()));
        }

        _ssids.clear();
        _auth_modes.clear();
        std::string options;
        for (const auto &record : records) {
            std::string ssid(reinterpret_cast<const char *>(record.ssid));
            if (ssid.empty() || std::find(_ssids.begin(), _ssids.end(), ssid) != _ssids.end()) {
                continue;
            }
            _ssids.push_back(ssid);
            _auth_modes.push_back(record.authmode);
            if (!options.empty()) {
                options += '\n';
            }
            options += ssid;
            options += (record.authmode == WIFI_AUTH_OPEN) ? "  [Open]" : "  [Secured]";
        }

        if (_ssids.empty()) {
            lv_dropdown_set_options(_network_dropdown, "No networks found");
            lv_label_set_text(_status_label, "No networks found");
        } else {
            lv_dropdown_set_options(_network_dropdown, options.c_str());
            lv_label_set_text_fmt(_status_label, "%u network(s) found", static_cast<unsigned>(_ssids.size()));
        }
    }

    void connectSelectedNetwork()
    {
        if (!_wifi_ready || _ssids.empty() || _network_dropdown == nullptr || _password_textarea == nullptr) {
            lv_label_set_text(_status_label, "Scan and select a network first");
            return;
        }

        uint16_t selected = lv_dropdown_get_selected(_network_dropdown);
        if (selected >= _ssids.size()) {
            lv_label_set_text(_status_label, "Invalid network selection");
            return;
        }

        const char *password = lv_textarea_get_text(_password_textarea);
        if (_auth_modes[selected] != WIFI_AUTH_OPEN && std::strlen(password) < 8) {
            lv_label_set_text(_status_label, "Password must be at least 8 characters");
            return;
        }

        wifi_config_t config = {};
        std::strncpy(reinterpret_cast<char *>(config.sta.ssid), _ssids[selected].c_str(), sizeof(config.sta.ssid) - 1);
        std::strncpy(reinterpret_cast<char *>(config.sta.password), password, sizeof(config.sta.password) - 1);
        config.sta.threshold.authmode = WIFI_AUTH_OPEN;
        config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

        esp_wifi_disconnect();
        esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &config);
        if (err == ESP_OK) {
            _save_credentials_on_connect.store(true);
            _connection_state.store(ConnectionState::CONNECTING);
            err = esp_wifi_connect();
        }

        if (err != ESP_OK) {
            _save_credentials_on_connect.store(false);
            _connection_state.store(ConnectionState::FAILED);
            lv_label_set_text_fmt(_status_label, "Connect failed: %s", esp_err_to_name(err));
        }
    }

    void updateStatusUi()
    {
        if (_status_label == nullptr) {
            return;
        }

        ConnectionState state = _connection_state.load();
        if (state == _last_rendered_state) {
            return;
        }
        _last_rendered_state = state;

        auto *status_bar = getSystem()->getDisplay().getStatusBar();
        switch (state) {
        case ConnectionState::DISCONNECTED:
            lv_label_set_text(_status_label, "Disconnected");
            status_bar->setWifiIconState(phone::StatusBar::WifiState::DISCONNECTED);
            break;
        case ConnectionState::CONNECTING:
            lv_label_set_text(_status_label, "Connecting...");
            status_bar->setWifiIconState(phone::StatusBar::WifiState::SIGNAL_1);
            break;
        case ConnectionState::CONNECTED:
            lv_label_set_text_fmt(_status_label, "Connected - IP: %s", _ip_address);
            status_bar->setWifiIconState(phone::StatusBar::WifiState::SIGNAL_3);
            break;
        case ConnectionState::FAILED:
            lv_label_set_text(_status_label, "Connection failed");
            status_bar->setWifiIconState(phone::StatusBar::WifiState::DISCONNECTED);
            break;
        case ConnectionState::UNKNOWN:
            break;
        }
    }

    bool _wifi_ready = false;
    esp_netif_t *_sta_netif = nullptr;
    esp_event_handler_instance_t _wifi_event_instance = nullptr;
    esp_event_handler_instance_t _ip_event_instance = nullptr;
    std::atomic<ConnectionState> _connection_state{ConnectionState::UNKNOWN};
    std::atomic<bool> _save_credentials_on_connect{false};
    ConnectionState _last_rendered_state = ConnectionState::UNKNOWN;
    char _ip_address[16] = {};
    std::vector<std::string> _ssids;
    std::vector<wifi_auth_mode_t> _auth_modes;
    lv_obj_t *_network_dropdown = nullptr;
    lv_obj_t *_password_textarea = nullptr;
    lv_obj_t *_status_label = nullptr;
    lv_obj_t *_keyboard = nullptr;
    lv_timer_t *_status_timer = nullptr;
    lv_obj_t *_notification = nullptr;
    lv_timer_t *_notification_timer = nullptr;
};

ESP_UTILS_REGISTER_PLUGIN_WITH_CONSTRUCTOR(
    systems::base::App,
    WifiApp,
    APP_NAME,
    []() {
        return std::shared_ptr<WifiApp>(WifiApp::requestInstance(), [](WifiApp *) {});
    }
)

} // namespace esp_brookesia::apps
