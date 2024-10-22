/*
 *  ESP32 WT32-SC01 Plus I2S Test
 *  I2S Audio Dac IC: NS4168
 */
#include <stdio.h>
#include <string.h>
#include <driver/i2s.h>
#include "driver/i2s_std.h"
#include "i2s_speaker.h"
#include "math.h"
#include "log.h"

// WT32-SC01 Pluse I2S 핀
#define CONFIG_I2S_BCK_PIN 36
#define CONFIG_I2S_LRCK_PIN 35
#define CONFIG_I2S_DATA_PIN 37

#define I2S_CHANNELS (2)
#define I2S_BUF_LEN (1024)

static const char* TAG = "I2S_AUDIO";

size_t bytes_written = 0;
static i2s_chan_handle_t tx_handle;

extern const unsigned short lack_korean_volume[27519];
extern const unsigned short over_korean_volume[33869];
extern const unsigned short normal_korean_volume[39163];
extern const unsigned char dingdong[120264];

int i2s_speak_init() {
  /* Get the default channel configuration by the helper macro.
   * This helper macro is defined in 'i2s_common.h' and shared by all the I2S communication modes.
   * It can help to specify the I2S role and port ID */
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
  /* Allocate a new TX channel and get the handle of this channel */
  i2s_new_channel(&chan_cfg, &tx_handle, NULL);

  /* Setting the configurations, the slot configuration and clock configuration can be generated by the macros
   * These two helper macros are defined in 'i2s_std.h' which can only be used in STD mode.
   * They can help to specify the slot and clock configurations for initialization or updating */
  i2s_std_config_t std_cfg = {
    //.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(48000),
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100),
    .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
    .gpio_cfg = {
        .mclk = I2S_GPIO_UNUSED,
        .bclk = CONFIG_I2S_BCK_PIN,
        .ws = CONFIG_I2S_LRCK_PIN,
        .dout = CONFIG_I2S_DATA_PIN,
        .din = I2S_GPIO_UNUSED,
        .invert_flags = {
            .mclk_inv = false,
            .bclk_inv = false,
            .ws_inv = false,
        },
    },
};
  /* Initialize the channel */
  i2s_channel_init_std_mode(tx_handle, &std_cfg);

  /* Before writing data, start the TX channel first */
  i2s_channel_enable(tx_handle);

  bytes_written = sizeof(normal_korean_volume);
  i2s_channel_write(tx_handle, normal_korean_volume, sizeof(normal_korean_volume), &bytes_written, portMAX_DELAY);
  bytes_written = sizeof(over_korean_volume);
  i2s_channel_write(tx_handle, over_korean_volume, sizeof(over_korean_volume), &bytes_written, portMAX_DELAY);
  bytes_written = sizeof(lack_korean_volume);
  i2s_channel_write(tx_handle, lack_korean_volume, sizeof(lack_korean_volume), &bytes_written, portMAX_DELAY);

  return 0;
}

void dingdong_volume() {
  if (i2s_channel_write(tx_handle, dingdong, sizeof(dingdong), &bytes_written, portMAX_DELAY) == ESP_OK) {
    LOGI(TAG, "play to Sound_effect_dingdong");
  }
}

void normal_volume() {
  if (i2s_channel_write(tx_handle, normal_korean_volume, sizeof(normal_korean_volume), &bytes_written, portMAX_DELAY) ==
      ESP_OK) {
    LOGI(TAG, "play to Normal_korean_volume");
  }
}
void over_volume() {
  if (i2s_channel_write(tx_handle, over_korean_volume, sizeof(over_korean_volume), &bytes_written, portMAX_DELAY) ==
      ESP_OK) {
    LOGI(TAG, "play to Over_korean_volume");
  }
}
void lack_volume() {
  if (i2s_channel_write(tx_handle, lack_korean_volume, sizeof(lack_korean_volume), &bytes_written, portMAX_DELAY) ==
      ESP_OK) {
    LOGI(TAG, "play to Lack_korean_volume");
  }
}
