set(req driver freertos hal esp_adc_cal log)

set(srcs)
set(includes)

if(CONFIG_I2C_FEATURE)
  list(APPEND srcs 
    "i2c/i2c_api.c" "i2c/esp32/i2c_hal.c")
  list(APPEND includes "./i2c")
endif()

if(CONFIG_GPIO_FEATURE)
  list(APPEND srcs 
    "gpio/gpio_api.c" "gpio/esp32/gpio_hal.c")
  list(APPEND includes "./gpio")
endif()

if(CONFIG_ADC_FEATURE)
  list(APPEND srcs 
    "adc/adc_api.c" "adc/esp32/adc_hal.c")
  list(APPEND includes "./adc")
endif()

if(CONFIG_DAC_FEATURE)
  list(APPEND srcs 
    "dac/dac_api.c" "dac/esp32/dac_hal.c")
  list(APPEND includes "./dac")
endif()

idf_component_register(
  SRCS "${srcs}"
  INCLUDE_DIRS ${includes}
  REQUIRES ${req}                    
)
