# ESP-IDF Arduino Hello World

This project is a minimal testbed for verifying that an Arduino sketch can be built as an ESP-IDF component.

It intentionally keeps only a simple `setup()` / `loop()` example:

- initialize `Serial`
- print `hello world`
- delay for one second

Sensor libraries and generated build artifacts are intentionally excluded. Add them locally only when an Arduino compatibility test needs them.
