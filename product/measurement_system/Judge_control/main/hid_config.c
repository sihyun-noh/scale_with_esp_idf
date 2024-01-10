#include "syslog.h"
#include "syscfg.h"
#include "hid_config.h"

#include <stdlib.h>
#include <string.h>

static const char *TAG = "hid_config";

static time_t generate_time(int hour, int min) {
  time_t now;
  struct tm timeinfo = { 0 };

  time(&now);
  localtime_r(&now, &timeinfo);

  timeinfo.tm_hour = hour;
  timeinfo.tm_min = min;

  LOGI(TAG, "Year:%d,Mon:%d,Hour:%d,Min:%d,Sec:%d", timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_hour,
       timeinfo.tm_min, timeinfo.tm_sec);

  return mktime(&timeinfo);
}

static time_t get_current_time(void) {
  time_t now;
  struct tm timeinfo = { 0 };

  time(&now);
  localtime_r(&now, &timeinfo);

  return mktime(&timeinfo);
}

bool validation_of_start_irrigation_time(time_t start_time, stage_t *p_stage) {
  // Get current time of hid and check if the irrigation start time is setting within two irrigation time zone
  // Generate a time of morning/evening irrigation time zone
  time_t curr_time = get_current_time();
  time_t start_time_of_morning = generate_time(5, 00);
  time_t end_time_of_morning = generate_time(9, 00);
  time_t start_time_of_evening = generate_time(17, 00);
  time_t end_time_of_evening = generate_time(21, 00);

  *p_stage = NONE_STAGE;
  if (start_time <= curr_time) {
    LOGI(TAG, "Start time = %ld is less than Curr time = %ld", start_time, curr_time);
    return false;
  }

  show_timestamp(curr_time);
  show_timestamp(start_time);
  show_timestamp(start_time_of_morning);
  show_timestamp(end_time_of_morning);
  show_timestamp(start_time_of_evening);
  show_timestamp(end_time_of_evening);

  if (curr_time < end_time_of_morning) {
    LOGI(TAG, "Currnt time is running on Morning time zone");
    if (start_time > start_time_of_morning && start_time <= end_time_of_morning) {
      LOGI(TAG, "Stage is current");
      *p_stage = CURR_STAGE;
      return true;
    } else if (start_time > start_time_of_evening && start_time <= end_time_of_evening) {
      LOGI(TAG, "Stage is next");
      *p_stage = NEXT_STAGE;
      return true;
    }
  } else if (curr_time > start_time_of_evening && curr_time <= end_time_of_evening) {
    LOGI(TAG, "Currnt time is running on Evening time zone");
    if (start_time > start_time_of_evening && start_time <= end_time_of_evening) {
      LOGI(TAG, "Stage is current");
      *p_stage = CURR_STAGE;
      return true;
    }
  } else if (curr_time > end_time_of_morning && curr_time < start_time_of_evening) {
    LOGI(TAG, "Stage is next");
    *p_stage = NEXT_STAGE;
    return true;
  }
  return false;
}

char *get_current_timestamp(void) {
  time_t now;
  static char timestamp[80] = { 0 };

  time(&now);
  struct tm *tm = localtime(&now);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm);
  return timestamp;
}

void show_timestamp(time_t now) {
  char timestamp[80] = { 0 };

  struct tm *tm = localtime(&now);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm);
  LOGI(TAG, "timestamp = %s", timestamp);
}

bool read_hid_config(config_value_t *cfg) {
  if (syscfg_get_blob(CFG_DATA, "hid_config", cfg, sizeof(config_value_t))) {
    LOGE(TAG, "Failed to read hid config, it seems there is no data!!!");
    return false;
  }
  return true;
}

bool save_hid_config(const char *flow, const char *start_time_hour, const char *start_time_minute, const char *zones) {
  config_value_t cfg;
  memset(&cfg, 0x00, sizeof(config_value_t));

  int flow_rate = 0;
  int i = 0, zone = 0;
  int hour = 0, min = 0;
  char *beg = NULL, *pch = NULL;

  if (!flow || !start_time_hour || !zones || strlen(flow) == 0 || strlen(start_time_hour) == 0 || strlen(zones) == 0) {
    LOGW(TAG, "flow and start_time and zone should be filled!!!");
    return false;
  }

  LOGI(TAG, "flow data = %s, zones = %s, start_time_hour = %s", flow, zones, start_time_hour);

  // Start Hour, Minute time
  hour = atoi(start_time_hour);
  min = atoi(start_time_minute);
  LOGI(TAG, "%d:%d", hour, min);
  time_t now = time(NULL);
  show_timestamp(now);
  time_t start_tm = generate_time(hour, min);
  show_timestamp(start_tm);
  cfg.start_time = start_tm;

  // Zone Areas
  if (zones) {
    beg = (char *)zones;
    pch = (char *)strpbrk(beg, ",");
    while (pch != NULL) {
      *pch = '\0';
      zone = atoi(beg);
      if (zone >= 1 && zone <= 6) {
        cfg.zones[i++] = zone;
      }
      beg = pch + 1;
      pch = (char *)strpbrk(beg, ",");
    }
    if (beg) {
      cfg.zones[i++] = atoi(beg);
    }
    cfg.zone_cnt = i;
    for (i = 0; i < cfg.zone_cnt; i++) {
      LOGI(TAG, "zones[%d] = %d", i, cfg.zones[i]);
    }
  }

  // Flow rate
  flow_rate = atoi(flow);
  if (flow_rate <= 0) {
    LOGW(TAG, "flow rate cannot be 0 or negative");
  } else {
    for (i = 0; i < cfg.zone_cnt; i++) {
      cfg.flow_rate[i] = flow_rate;
    }
  }
  syscfg_set_blob(CFG_DATA, "hid_config", &cfg, sizeof(config_value_t));
  return true;
}
