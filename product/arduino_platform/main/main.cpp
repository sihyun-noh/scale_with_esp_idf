#include "Arduino.h"
// #include "RTClib.h"
#include "Wire.h"

const int i2c_sda = 1;
const int i2c_scl = 2;

// RTC_DS3231 rtc;
int status = 0;

void Scanner() {
  Serial.println();
  Serial.println("I2C scanner. Scanning ...");
  byte count = 0;

  Wire.begin();
  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);        // Begin I2C transmission Address (i)
    if (Wire.endTransmission() == 0)  // Receive 0 = success (ACK response)
    {
      Serial.print("Found address: ");
      Serial.print(i, DEC);
      Serial.print(" (0x");
      Serial.print(i, HEX);  // PCF8574 7 bit address
      Serial.println(")");
      count++;
    }
  }
  Serial.print("Found ");
  Serial.print(count, DEC);  // numbers of devices
  Serial.println(" device(s).");
}

extern "C" void app_main(void) {
  Serial.begin(115200);
  Serial.println("hello world");
  Wire.begin(i2c_sda, i2c_scl);
  // rtc.begin();
  while (1) {
    Scanner();
    delay(1000);
  }
}
