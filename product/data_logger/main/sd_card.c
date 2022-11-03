#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "config.h"
#include "log.h"
#include "sysevent.h"
#include "event_ids.h"

#define MOUNT_POINT "/sdcard"

static const char *TAG = "sdcard";

sdmmc_card_t *card;
sdmmc_host_t host;
const char mount_point[] = MOUNT_POINT;

void sdcard_init(void) {
  esp_err_t ret;

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 5,
    .allocation_unit_size = 16 * 1024
  };
  LOGI(TAG, "Initializing SD card");

  host = (sdmmc_host_t)SDSPI_HOST_DEFAULT();
  spi_bus_config_t bus_cfg = {
    .mosi_io_num = SDCARD_SPI_MOSI,
    .miso_io_num = SDCARD_SPI_MISO,
    .sclk_io_num = SDCARD_SPI_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 4000,
  };
  ret = spi_bus_initialize(host.slot, &bus_cfg, SDCARD_SPI_DMA_CHAN);
  if (ret != ESP_OK) {
    LOGE(TAG, "Failed to initialize bus.");
    return;
  }

  // This initializes the slot without card detect (CD) and write protect (WP) signals.
  // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = SDCARD_SPI_CS;
  slot_config.host_id = host.slot;

  LOGI(TAG, "Mounting filesystem");
  ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

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
    }
    return;
  }
  LOGI(TAG, "Filesystem mounted");

  // Card has been initialized, print its properties
  sdmmc_card_print_info(stdout, card);
}

#if (SENSOR_TYPE == SHT3X)
void write_temperature_humidity(void)
{
  char s_temperature[20] = { 0 };
  char s_humidity[20] = { 0 };
  const char *file_sensor = MOUNT_POINT"/sht3x.csv";

  LOGI(TAG, "Opening file %s", file_sensor);
  FILE *f = fopen(file_sensor, "a");
  if (f == NULL) {
    LOGE(TAG, "Failed to open file for writing");
    return;
  }
  sysevent_get(SYSEVENT_BASE, I2C_HUMIDITY_EVENT, &s_humidity, sizeof(s_humidity));
  sysevent_get(SYSEVENT_BASE, I2C_TEMPERATURE_EVENT, &s_temperature, sizeof(s_temperature));
  if (s_temperature[0] && s_humidity[0]) {
    fprintf(f, "%s,%s\n", s_temperature, s_humidity);
  }

  fclose(f);
}
#endif

#if (SENSOR_TYPE == SCD4X)
void write_co2_temperature_humidity(void)
{
  char s_co2[20] = { 0 };
  char s_temperature[20] = { 0 };
  char s_humidity[20] = { 0 };
  const char *file_sensor = MOUNT_POINT"/scd4x.csv";

  LOGI(TAG, "Opening file %s", file_sensor);
  FILE *f = fopen(file_sensor, "a");
  if (f == NULL) {
    LOGE(TAG, "Failed to open file for writing");
    return;
  }
  sysevent_get(SYSEVENT_BASE, I2C_CO2_EVENT, &s_co2, sizeof(s_co2));
  sysevent_get(SYSEVENT_BASE, I2C_HUMIDITY_EVENT, &s_humidity, sizeof(s_humidity));
  sysevent_get(SYSEVENT_BASE, I2C_TEMPERATURE_EVENT, &s_temperature, sizeof(s_temperature));
  if (s_co2[0] && s_temperature[0] && s_humidity[0]) {
    fprintf(f, "%s,%s,%s\n", s_co2, s_temperature, s_humidity);
  }

  fclose(f);
}
#endif

#if (SENSOR_TYPE == RK520_02)
void write_rika_soil_ec(void)
{
  char s_bulk_ec[20] = { 0 };
  char s_saturation_ec[20] = { 0 };
  char s_moisture[20] = { 0 };
  char s_temperature[20] = { 0 };
  const char *file_sensor = MOUNT_POINT"/rk520_02.csv";

  LOGI(TAG, "Opening file %s", file_sensor);
  FILE *f = fopen(file_sensor, "a");
  if (f == NULL) {
    LOGE(TAG, "Failed to open file for writing");
    return;
  }
  sysevent_get(SYSEVENT_BASE, MB_SOIL_EC_EVENT, s_saturation_ec, sizeof(s_saturation_ec));
  sysevent_get(SYSEVENT_BASE, MB_SOIL_BULK_EC_EVENT, s_bulk_ec, sizeof(s_bulk_ec));
  sysevent_get(SYSEVENT_BASE, MB_MOISTURE_EVENT, s_moisture, sizeof(s_moisture));
  sysevent_get(SYSEVENT_BASE, MB_TEMPERATURE_EVENT, s_temperature, sizeof(s_temperature));
  if (s_saturation_ec[0] && s_temperature[0] && s_moisture[0]) {
    fprintf(f, "%s,%s,%s\n", s_saturation_ec, s_moisture, s_temperature);
  }

  fclose(f);
}
#endif

#if (SENSOR_TYPE == RK500_02)
void write_rika_water_ph(void)
{
  char s_ph[20] = { 0 };
  const char *file_sensor = MOUNT_POINT"/rk500_02.csv";

  LOGI(TAG, "Opening file %s", file_sensor);
  FILE *f = fopen(file_sensor, "a");
  if (f == NULL) {
    LOGE(TAG, "Failed to open file for writing");
    return;
  }
  sysevent_get(SYSEVENT_BASE, MB_WATER_PH_EVENT, s_ph, sizeof(s_ph));
  if (s_ph[0]) {
    fprintf(f, "%s\n", s_ph);
  }

  fclose(f);
}
#endif

#if (SENSOR_TYPE == SWSR7500)
void write_solar_radiation(void)
{
  char s_pyranometer[20] = { 0 };
  const char *file_sensor = MOUNT_POINT"/swsr7500.csv";

  LOGI(TAG, "Opening file %s", file_sensor);
  FILE *f = fopen(file_sensor, "a");
  if (f == NULL) {
    LOGE(TAG, "Failed to open file for writing");
    return;
  }
  sysevent_get(SYSEVENT_BASE, MB_PYRANOMETER_EVENT, s_pyranometer, sizeof(s_pyranometer));
  if (s_pyranometer[0]) {
    fprintf(f, "%s\n", s_pyranometer);
  }

  fclose(f);
}
#endif

#if (SENSOR_TYPE == ATLAS_PH)
void write_atlas_water_ph(void)
{
  char s_ph[20] = { 0 };
  const char *file_sensor = MOUNT_POINT"/atlas_ph.csv";

  LOGI(TAG, "Opening file %s", file_sensor);
  FILE *f = fopen(file_sensor, "a");
  if (f == NULL) {
    LOGE(TAG, "Failed to open file for writing");
    return;
  }
  sysevent_get(SYSEVENT_BASE, I2C_PH_EVENT, s_ph, sizeof(s_ph));
  if (s_ph[0]) {
    fprintf(f, "%s\n", s_ph);
  }

  fclose(f);
}
#endif

#if (SENSOR_TYPE == ATLAS_EC)
void write_atlas_water_ec(void)
{
  char s_ec[20] = { 0 };
  const char *file_sensor = MOUNT_POINT"/atlas_ec.csv";

  LOGI(TAG, "Opening file %s", file_sensor);
  FILE *f = fopen(file_sensor, "a");
  if (f == NULL) {
    LOGE(TAG, "Failed to open file for writing");
    return;
  }
  sysevent_get(SYSEVENT_BASE, I2C_EC_EVENT, s_ec, sizeof(s_ec));
  if (s_ec[0]) {
    fprintf(f, "%s\n", s_ec);
  }

  fclose(f);
}
#endif

#if (SENSOR_TYPE == RK110_02)
void write_wind_direction(void)
{
  char s_wind_direction[20] = { 0 };
  const char *file_sensor = MOUNT_POINT"/rk110_02.csv";

  LOGI(TAG, "Opening file %s", file_sensor);
  FILE *f = fopen(file_sensor, "a");
  if (f == NULL) {
    LOGE(TAG, "Failed to open file for writing");
    return;
  }
  sysevent_get(SYSEVENT_BASE, MB_WIND_DIRECTION_EVENT, s_wind_direction, sizeof(s_wind_direction));
  if (s_wind_direction[0]) {
    fprintf(f, "%s\n", s_wind_direction);
  }

  fclose(f);
}
#endif

#if (SENSOR_TYPE == RK100_02)
void write_wind_speed(void)
{
  char s_wind_speed[20] = { 0 };
  const char *file_sensor = MOUNT_POINT"/rk100_02.csv";

  LOGI(TAG, "Opening file %s", file_sensor);
  FILE *f = fopen(file_sensor, "a");
  if (f == NULL) {
    LOGE(TAG, "Failed to open file for writing");
    return;
  }
  sysevent_get(SYSEVENT_BASE, MB_WIND_SPEED_EVENT, s_wind_speed, sizeof(s_wind_speed));
  if (s_wind_speed[0]) {
    fprintf(f, "%s\n", s_wind_speed);
  }

  fclose(f);
}
#endif

#if (SENSOR_TYPE == RK500_13)
void write_rika_water_ec(void)
{
  char s_ec[20] = { 0 };
  char s_temperature[20] = { 0 };
  const char *file_sensor = MOUNT_POINT"/rk500_13.csv";

  LOGI(TAG, "Opening file %s", file_sensor);
  FILE *f = fopen(file_sensor, "a");
  if (f == NULL) {
    LOGE(TAG, "Failed to open file for writing");
    return;
  }
  sysevent_get(SYSEVENT_BASE, MB_WATER_EC_EVENT, s_ec, sizeof(s_ec));
  sysevent_get(SYSEVENT_BASE, MB_TEMPERATURE_EVENT, s_temperature, sizeof(s_temperature));
  if (s_ec[0] && s_temperature[0]) {
    fprintf(f, "%s,%s\n", s_ec, s_temperature);
  }

  fclose(f);
}
#endif

void sdcard_write_data(void) {
#if (SENSOR_TYPE == SHT3X)
  write_temperature_humidity();
#elif (SENSOR_TYPE == SCD4X)
  write_co2_temperature_humidity();
#elif (SENSOR_TYPE == RK520_02)
  write_rika_soil_ec();
#elif (SENSOR_TYPE == RK500_02)
  write_rika_water_ph();
#elif (SENSOR_TYPE == SWSR7500)
  write_solar_radiation();
#elif (SENSOR_TYPE == ATLAS_PH)
  write_atlas_water_ph();
#elif (SENSOR_TYPE == ATLAS_EC)
  write_atlas_water_ec();
#elif (SENSOR_TYPE == RK110_02)
  write_wind_direction();
#elif (SENSOR_TYPE == RK100_02)
  write_wind_speed();
#elif (SENSOR_TYPE == RK500_13)
  write_rika_water_ec();
#endif

  LOGI(TAG, "File written");

  // All done, unmount partition and disable SPI peripheral
  esp_vfs_fat_sdcard_unmount(mount_point, card);
  LOGI(TAG, "Card unmounted");

  // deinitialize the bus after all devices are removed
  spi_bus_free(host.slot);
}
