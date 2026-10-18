#include "dajilog_product_start.h"

#include <ctime>
#include <new>
#include <tuple>
#include <vector>

#include "dajilog_board.h"
#include "esp_brookesia.hpp"
#include "esp_lib_utils.h"
#include "esp_ota_ops.h"

#ifdef ESP_UTILS_LOG_TAG
#undef ESP_UTILS_LOG_TAG
#endif
#define ESP_UTILS_LOG_TAG "DajilogStart"

using namespace esp_brookesia;
using namespace esp_brookesia::gui;
using namespace esp_brookesia::systems::phone;

static const Stylesheet *select_stylesheet(const dajilog_board_info_t *board)
{
    if (board == nullptr) {
        return nullptr;
    }

    switch (board->ui_profile) {
    case DAJILOG_UI_PROFILE_PORTRAIT_720_1280:
        return &STYLESHEET_720_1280_DARK;
    case DAJILOG_UI_PROFILE_LANDSCAPE_1024_600:
        return &STYLESHEET_1024_600_DARK;
    default:
        return nullptr;
    }
}

extern "C" void dajilog_product_start(void)
{
    const dajilog_board_info_t *board = dajilog_board_get_info();
    ESP_UTILS_CHECK_NULL_EXIT(board, "Board information is unavailable");
    ESP_UTILS_LOGI("Starting on %s (%ux%u)", board->board_name,
                   board->display_width, board->display_height);

    ESP_UTILS_CHECK_ERROR_EXIT(dajilog_board_init(), "Board init failed");
    ESP_UTILS_CHECK_ERROR_EXIT(dajilog_board_display_start(), "Display start failed");

    LvLock::registerCallbacks(
        [](int timeout_ms) {
            return dajilog_board_display_lock(timeout_ms);
        },
        []() {
            dajilog_board_display_unlock();
            return true;
        });

    lv_obj_t *splash = nullptr;
    {
        LvLockGuard gui_guard;
        splash = lv_obj_create(nullptr);
        lv_obj_set_style_bg_color(splash, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(splash, LV_OPA_COVER, LV_PART_MAIN);

        lv_obj_t *title = lv_label_create(splash);
        lv_label_set_text(title, "DAJILOG SYSTEM");
        lv_obj_set_style_text_color(title, lv_color_hex(0x111111), LV_PART_MAIN);
        lv_obj_set_style_text_font(title, &lv_font_montserrat_40, LV_PART_MAIN);
        lv_obj_center(title);
        lv_screen_load(splash);
    }
    ESP_UTILS_CHECK_ERROR_EXIT(dajilog_board_backlight_on(), "Backlight failed");

    Phone *phone = new (std::nothrow) Phone();
    ESP_UTILS_CHECK_NULL_EXIT(phone, "Create phone failed");

    const Stylesheet *selected = select_stylesheet(board);
    if (selected != nullptr) {
        Stylesheet *stylesheet = new (std::nothrow) Stylesheet(*selected);
        ESP_UTILS_CHECK_NULL_EXIT(stylesheet, "Create stylesheet failed");
        ESP_UTILS_CHECK_FALSE_EXIT(phone->addStylesheet(stylesheet), "Add stylesheet failed");
        ESP_UTILS_CHECK_FALSE_EXIT(phone->activateStylesheet(stylesheet), "Activate stylesheet failed");
        delete stylesheet;
    }

    {
        LvLockGuard gui_guard;
        ESP_UTILS_CHECK_FALSE_EXIT(phone->begin(), "Begin phone failed");

        std::vector<systems::base::Manager::RegistryAppInfo> apps;
        ESP_UTILS_CHECK_FALSE_EXIT(phone->initAppFromRegistry(apps), "Init apps failed");
        ESP_UTILS_LOGI("Registry contains %u app(s)", static_cast<unsigned>(apps.size()));
        for (const auto &app_info : apps) {
            ESP_UTILS_LOGI("Registry app: %s", std::get<0>(app_info).c_str());
        }
        ESP_UTILS_CHECK_FALSE_EXIT(phone->installAppFromRegistry(apps), "Install apps failed");

        if (splash != nullptr && splash != lv_screen_active()) {
            lv_obj_delete(splash);
            splash = nullptr;
        }

        lv_timer_create(
            [](lv_timer_t *timer) {
                time_t now;
                struct tm timeinfo;
                Phone *active_phone = static_cast<Phone *>(timer->user_data);
                if (active_phone == nullptr) {
                    return;
                }
                time(&now);
                localtime_r(&now, &timeinfo);
                active_phone->getDisplay().getStatusBar()->setClock(
                    timeinfo.tm_hour, timeinfo.tm_min);
            },
            1000, phone);
    }

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (running != nullptr &&
        esp_ota_get_state_partition(running, &ota_state) == ESP_OK &&
        ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ota_mark_app_valid_cancel_rollback());
    }
}
