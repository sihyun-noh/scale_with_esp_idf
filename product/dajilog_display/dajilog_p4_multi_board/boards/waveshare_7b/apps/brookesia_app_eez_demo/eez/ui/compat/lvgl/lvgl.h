#pragma once

/*
 * EEZ Studio generates #include <lvgl/lvgl.h>, while ESP-IDF's managed
 * LVGL component exports lvgl.h directly.  Keep this compatibility shim
 * outside generated/ so regenerating the UI never overwrites it.
 */
#include <lvgl.h>
