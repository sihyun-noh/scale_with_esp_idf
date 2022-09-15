#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "log.h"
#include "syscfg.h"
#include "nvs_flash.h"
#include "nvs.h"

#define KEY "cacert_blob"
static const char *TAG = "nuit_syscfg_test";

char *cacert =
    "-----EGIN CERTIFICATE-----\
MIIDKzCCAhOgAwIBAgIUBxM3WJf2bP12kAfqhmhhjZWv0ukwDQYJKoZIhvcNAQEL\
BQAwJTEjMCEGA1UEAwwaRVNQMzIgSFRUUFMgc2VydmVyIGV4YW1wbGUwHhcNMTgx\
MDE3MTEzMjU3WhcNMjgxMDE0MTEzMjU3WjAlMSMwIQYDVQQDDBpFU1AzMiBIVFRQ\
UyBzZXJ2ZXIgZXhhbXBsZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\
ALBint6nP77RCQcmKgwPtTsGK0uClxg+LwKJ3WXuye3oqnnjqJCwMEneXzGdG09T\
sA0SyNPwrEgebLCH80an3gWU4pHDdqGHfJQa2jBL290e/5L5MB+6PTs2NKcojK/k\
qcZkn58MWXhDW1NpAnJtjVniK2Ksvr/YIYSbyD+JiEs0MGxEx+kOl9d7hRHJaIzd\
GF/vO2pl295v1qXekAlkgNMtYIVAjUy9CMpqaQBCQRL+BmPSJRkXBsYk8GPnieS4\
sUsp53DsNvCCtWDT6fd9D1v+BB6nDk/FCPKhtjYOwOAZlX4wWNSZpRNr5dfrxKsb\
jAn4PCuR2akdF4G8WLUeDWECAwEAAaNTMFEwHQYDVR0OBBYEFMnmdJKOEepXrHI/\
ivM6mVqJgAX8MB8GA1UdIwQYMBaAFMnmdJKOEepXrHI/ivM6mVqJgAX8MA8GA1Ud\
EwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBADiXIGEkSsN0SLSfCF1VNWO3\
emBurfOcDq4EGEaxRKAU0814VEmU87btIDx80+z5Dbf+GGHCPrY7odIkxGNn0DJY\
W1WcF+DOcbiWoUN6DTkAML0SMnp8aGj9ffx3x+qoggT+vGdWVVA4pgwqZT7Ybntx\
bkzcNFW0sqmCv4IN1t4w6L0A87ZwsNwVpre/j6uyBw7s8YoJHDLRFT6g7qgn0tcN\
ZufhNISvgWCVJQy/SZjNBHSpnIdCUSJAeTY2mkM4sGxY0Widk8LnjydxZUSxC3Nl\
hb6pnMh3jRq4h0+5CZielA4/a+TdrNPv/qok67ot/XJdY3qHCCd8O2b14OVq9jo=\
-----END CERTIFICATE-----\
";
char load_max_buf[2048] = { 0 };
size_t total = 0;
int ret = 0;

TEST_CASE("syscfg_show", "[syscfg_show]") {
  LOGI(TAG, "CFG_DATA");
  TEST_ASSERT_EQUAL(0, syscfg_show(CFG_DATA));
  LOGI(TAG, "MFG_DATA");
  TEST_ASSERT_EQUAL(0, syscfg_show(MFG_DATA));
}

TEST_CASE("syscfg_set_blob", "[syscfg_set_blob]") {
  TEST_ASSERT_EQUAL(0, syscfg_set_blob(CFG_DATA, KEY, cacert, strlen(cacert)));
}

TEST_CASE("syscfg_get_blob", "[syscfg_get_blob]") {
  TEST_ASSERT_EQUAL(0, syscfg_get_blob(CFG_DATA, KEY, load_max_buf, strlen(cacert)));
  LOGI(TAG, "%s", load_max_buf);
  if ((ret = strncmp(cacert, load_max_buf, strlen(load_max_buf))) != 0) {
    LOGE(TAG, "not string equivalence!");
  }
}

TEST_CASE("syscfg_get_blob_size", "[syscfg_get_blob_size]") {
  size_t size;
  size = syscfg_get_blob_size(CFG_DATA, KEY);
  LOGI(TAG, "blob size : %lu", size);
}

TEST_CASE("syscfg_unset", "[syscfg_unset]") {
  TEST_ASSERT_EQUAL(0, syscfg_unset(CFG_DATA, KEY));
}

TEST_CASE("syscfg_set", "[syscfg_set]") {
  TEST_ASSERT_EQUAL(0, syscfg_set(MFG_DATA, "blob", "bbbbb"));
}

TEST_CASE("syscfg_get", "[syscfg_get]") {
  char str[30] = { 0 };
  TEST_ASSERT_EQUAL(0, syscfg_get(MFG_DATA, "blob", str, sizeof(str)));
  LOGI(TAG, "blob : %s", str);
}
