#include <memory>

#include "esp_brookesia.hpp"
#include "esp_lib_utils.h"
#include "systems/phone/esp_brookesia_phone_app.hpp"
#include "eez/ui/eez_ui_port.h"

#define APP_NAME "EEZ Demo"

using namespace esp_brookesia::systems;

namespace esp_brookesia::apps {

class EezDemoApp : public phone::App {
public:
    static EezDemoApp *requestInstance()
    {
        static EezDemoApp instance;
        return &instance;
    }

protected:
    EezDemoApp(): App(APP_NAME, nullptr, false, true, true)
    {
    }

    bool run() override
    {
        return eez_ui_port_create();
    }

    bool back() override
    {
        return notifyCoreClosed();
    }

    bool cleanResource() override
    {
        // Brookesia deletes recorded screens, timers and animations after this callback.
        // Clear generated-code globals here so the app can be opened again safely.
        eez_ui_port_prepare_cleanup();
        return true;
    }
};

ESP_UTILS_REGISTER_PLUGIN_WITH_CONSTRUCTOR(
    systems::base::App,
    EezDemoApp,
    APP_NAME,
    []() {
        return std::shared_ptr<EezDemoApp>(EezDemoApp::requestInstance(), [](EezDemoApp *) {});
    }
)

} // namespace esp_brookesia::apps
