#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println("hello world");
}

void loop() {
  Serial.println("hello world");
  delay(1000);
}

extern "C" void app_main() {
  initArduino();
  setup();

  while (true) {
    loop();
  }
}
