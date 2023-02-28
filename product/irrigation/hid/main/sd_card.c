#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "config.h"
#include "log.h"
#include "sysevent.h"
#include "event_ids.h"
#include "sys_status.h"

#define MOUNT_POINT "/sdcard"

static const char *TAG = "sdcard";

sdmmc_card_t *card;
sdmmc_host_t host;
const char mount_point[] = MOUNT_POINT;

void sdcard_init(void) {
  esp_err_t ret;

  LOGI(TAG, "Initializing SD card");

  host = (sdmmc_host_t)SDSPI_HOST_DEFAULT();
  host.slot = SDSPI_HOST_ID;
  spi_bus_config_t bus_cfg = {
    .mosi_io_num = SD_MOSI,
    .miso_io_num = SD_MISO,
    .sclk_io_num = SD_SCLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 4000,
  };

  ret = spi_bus_initialize(host.slot, &bus_cfg, SDCARD_SPI_DMA_CHAN);
  if (ret != ESP_OK) {
    LOGE(TAG, "Failed to initialize bus");
  } else {
    LOGI(TAG, "Success to initialize bus");
  }
}

int sdcard_mount(void) {
  esp_vfs_fat_sdmmc_mount_config_t mount_config = { .format_if_mount_failed = false,
                                                    .max_files = 5,
                                                    .allocation_unit_size = 16 * 1024 };
  // This initializes the slot without card detect (CD) and write protect (WP) signals.
  // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = SD_CS;
  slot_config.host_id = host.slot;

  LOGI(TAG, "Mounting filesystem");
  esp_err_t ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      LOGE(TAG,
           "Failed to mount filesystem. "
           "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
    } else {
      LOGE(TAG,
           "Failed to initialize the card (%s). "
           "Make sure SD card lines have pull-up resistors in place.",
           esp_err_to_name(ret));

      set_sdcard_fail(1);
    }
    return -1;
  }
  LOGI(TAG, "Filesystem mounted");

  // Card has been initialized, print its properties
  sdmmc_card_print_info(stdout, card);
  return 0;
}

int sdcard_unmount(void) {
  if (ESP_OK == esp_vfs_fat_sdmmc_unmount()) {
    return 0;
  }
  return -1;
}

int sdcard_get_status(void) {
  if (ESP_OK != sdmmc_get_status(card)) {
    return -1;
  }
  return 0;
}

void write_rtc_time(FILE *f) {
  struct tm time = { 0 };

  // rtc_get_time(&time);
  fprintf(f, "%04d-%02d-%02d,%02d:%02d:%02d, ", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour,
          time.tm_min, time.tm_sec);
  LOGI(TAG, "%04d-%02d-%02d,%02d:%02d:%02d, ", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour,
       time.tm_min, time.tm_sec);
}

void sdcard_write_data(void) {
  // write_water_supply_data();

  LOGI(TAG, "File written");

  // All done, unmount partition and disable SPI peripheral
  // esp_vfs_fat_sdcard_unmount(mount_point, card);
  // LOGI(TAG, "Card unmounted");

  // deinitialize the bus after all devices are removed
  // spi_bus_free(host.slot);
}
