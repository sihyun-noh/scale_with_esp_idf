#include "eez_ui_flow_lifecycle.h"

#include "esp_log.h"
#include "generated/brookesia_dajilog/src/ui/eez-flow.h"

namespace {

const char *TAG = "EezUiFlow";

} // namespace

extern "C" void eez_ui_flow_stop(void)
{
    if (eez::flow::isFlowStopped()) {
        return;
    }

    // stop() requests shutdown; one tick performs the actual queue, watch and
    // FlowState cleanup. LVGL screens are still valid at this point.
    eez::flow::stop();
    eez::flow::tick();
    ESP_LOGI(TAG, "EEZ Flow stopped");
}
