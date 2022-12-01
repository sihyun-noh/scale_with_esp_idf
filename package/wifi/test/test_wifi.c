#include "string.h"
#include "unity.h"
#include "wifi_manager.h"
#include "log.h"

static const char *TAG = "unity_wifi_test";

TEST_CASE("test to wifi_init().", "[wifi_init]") {
  TEST_ASSERT_EQUAL(0, wifi_user_init());
  LOGI(TAG, "TEST OK");
}

TEST_CASE("test to dewifi_init().", "[wifi_deinit]") {
  TEST_ASSERT_EQUAL(0, wifi_user_deinit());
  LOGI(TAG, "TEST OK");
}

TEST_CASE("test to wifi_ap_mode().", "[wifi_ap]") {
  TEST_ASSERT_EQUAL(0, wifi_ap_mode("TEST_AP_MODE", "1234567qwe"));
  LOGI(TAG, "TEST OK");
}

TEST_CASE("test to wifi_sta_mode().", "[wifi_sta]") {
  TEST_ASSERT_EQUAL(0, wifi_sta_mode());
  LOGI(TAG, "TEST OK");
}

TEST_CASE("test to wifi_stop().", "[wifi_stop]") {
  TEST_ASSERT_EQUAL(0, wifi_stop_mode());
  LOGI(TAG, "TEST OK");
}

TEST_CASE("test to wifi_connect().", "[wifi_connect]") {
  TEST_ASSERT_EQUAL(0, wifi_connect_ap("COMFAST_TEST_01", "admin123"));
  LOGI(TAG, "TEST OK");
}

TEST_CASE("test to wifi_diconnect().", "[wifi_diconnect]") {
  TEST_ASSERT_EQUAL(0, wifi_disconnect_ap());
  LOGI(TAG, "TEST OK");
}
