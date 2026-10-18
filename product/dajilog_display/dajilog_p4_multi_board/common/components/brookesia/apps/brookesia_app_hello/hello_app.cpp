#include <memory>

#include "lvgl.h"
#include "esp_brookesia.hpp"
#include "esp_log.h"
#include "esp_lib_utils.h"
#include "systems/phone/esp_brookesia_phone_app.hpp"

#define APP_NAME "Hello Test"

using namespace esp_brookesia::systems;

namespace esp_brookesia::apps {

class HelloApp : public phone::App {
public:
    static HelloApp *requestInstance()
    {
        static HelloApp instance;
        return &instance;
    }

protected:
    HelloApp(): App(APP_NAME, nullptr, true, true, true)
    {
    }

    bool run() override
    {
        ESP_LOGI(APP_NAME, "run() entered");
        lv_obj_t *label = lv_label_create(lv_scr_act());
        lv_label_set_text(label, "Hello from Custom App");
        lv_obj_center(label);
        return true;
    }

    bool back() override
    {
        return notifyCoreClosed();
    }
};

ESP_UTILS_REGISTER_PLUGIN_WITH_CONSTRUCTOR(
    systems::base::App,
    HelloApp,
    APP_NAME,
    []() {
        return std::shared_ptr<HelloApp>(HelloApp::requestInstance(), [](HelloApp *) {});
    }
)

} // namespace esp_brookesia::apps
