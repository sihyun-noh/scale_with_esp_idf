#include <memory>
#include <cstdio>
#include "lvgl.h"
#include "esp_brookesia.hpp"
#include "esp_lib_utils.h"
#include "systems/phone/esp_brookesia_phone_app.hpp"
#include "dajilog_board.h"
#include "dajilog_can.h"
#include "dajilog_can_device_manager.h"
#include "dajilog_can_tr07e.h"

#define APP_NAME "CAN Diagnostic"
using namespace esp_brookesia::systems;
namespace esp_brookesia::apps {
class CanDiagnosticApp : public phone::App {
  public:
  static CanDiagnosticApp* requestInstance() {
    static CanDiagnosticApp instance;
    return &instance;
  }

  protected:
  CanDiagnosticApp() : App(APP_NAME, nullptr, true, true, true) {}
  bool run() override {
    lv_obj_t* screen = lv_scr_act();
    title_ = lv_label_create(screen);
    lv_label_set_text(title_, "CAN Diagnostic");
    lv_obj_align(title_, LV_ALIGN_TOP_MID, 0, 32);
    status_ = lv_label_create(screen);
    lv_label_set_text(status_, "Starting CAN...");
    lv_obj_set_width(status_, LV_PCT(90));
    lv_label_set_long_mode(status_, LV_LABEL_LONG_WRAP);
    lv_obj_center(status_);
    const dajilog_board_can_config_t* b = dajilog_board_get_can_config();
    if (!b || !b->supported || b->tx_gpio < 0 || b->rx_gpio < 0) {
      lv_label_set_text(status_, "CAN GPIO is not configured.\nSet TX/RX in menuconfig.");
      return true;
    }
    esp_err_t e = ESP_OK;
    if (!service_started_) {
      dajilog_can_config_t c = { .tx_gpio = b->tx_gpio,
                                 .rx_gpio = b->rx_gpio,
                                 .bitrate = b->bitrate,
                                 .tx_queue_length = 16,
                                 .rx_queue_length = 32,
                                 .filter = dajilog_can_filter_accept_all() };
      e = dajilog_can_init(&c);
      if (e == ESP_OK)
        e = dajilog_can_start();
      if (e == ESP_OK)
        e = dajilog_can_device_manager_start();
      if (e == ESP_OK)
        service_started_ = true;
    }
    if (e != ESP_OK) {
      lv_label_set_text_fmt(status_, "CAN start failed: %s", esp_err_to_name(e));
      return true;
    }
    timer_ = lv_timer_create([](lv_timer_t* t) { static_cast<CanDiagnosticApp*>(t->user_data)->refresh(); }, 200, this);
    refresh();
    return true;
  }
  bool back() override { return notifyCoreClosed(); }
  bool cleanResource() override {
    if (timer_)
      lv_timer_delete(timer_);
    timer_ = nullptr;
    title_ = nullptr;
    status_ = nullptr;
    return true;
  }

  private:
  void refresh() {
    if (!status_)
      return;
    dajilog_can_status_t bus = {};
    const dajilog_can_device_config_t* cfg = nullptr;
    dajilog_can_device_state_t state = {};
    char text[512];
    if (dajilog_can_get_status(&bus) != ESP_OK) {
      lv_label_set_text(status_, "CAN status unavailable");
      return;
    }
    int n =
        snprintf(text, sizeof(text),
                 "Bus: %s  GPIO TX:%d RX:%d\n"
                 "RX total:%lu  queued:%lu  missed:%lu  errors:%lu\n"
                 "Last: 0x%03lX  %s  DLC:%u\n\n",
                 bus.state == DAJILOG_CAN_RUNNING ? "RUNNING" : "NOT RUNNING", dajilog_board_get_can_config()->tx_gpio,
                 dajilog_board_get_can_config()->rx_gpio, (unsigned long)bus.total_rx, (unsigned long)bus.pending_rx,
                 (unsigned long)bus.rx_missed, (unsigned long)bus.bus_errors, (unsigned long)bus.last_rx_id,
                 bus.last_rx_extended ? "EXT" : "STD", (unsigned)bus.last_rx_dlc);
    if (dajilog_can_device_manager_count() > 0 && dajilog_can_device_manager_get(0, &cfg, &state) == ESP_OK) {
      const auto* m = reinterpret_cast<const dajilog_tr07e_measurement_t*>(state.decoded);
      snprintf(text + n, sizeof(text) - (size_t)n,
               "%s [%s]\nID: 0x%03lX  Frames: %lu\nSpeed: %.2f m/s\nRCS: %u dBm\nDirection: %s", cfg->display_name,
               state.online ? "ONLINE" : "OFFLINE", (unsigned long)cfg->status_id, (unsigned long)state.rx_count,
               (double)m->speed_mps, m->rcs_dbm, m->reverse ? "Reverse" : "Forward");
    }
    lv_label_set_text(status_, text);
  }
  static inline bool service_started_ = false;
  lv_obj_t* title_ = nullptr;
  lv_obj_t* status_ = nullptr;
  lv_timer_t* timer_ = nullptr;
};
ESP_UTILS_REGISTER_PLUGIN_WITH_CONSTRUCTOR(systems::base::App, CanDiagnosticApp, APP_NAME, [] {
  return std::shared_ptr<CanDiagnosticApp>(CanDiagnosticApp::requestInstance(), [](CanDiagnosticApp*) {});
})
}  // namespace esp_brookesia::apps
