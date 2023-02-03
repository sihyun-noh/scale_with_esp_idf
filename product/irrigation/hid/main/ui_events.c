// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.1.1
// LVGL VERSION: 8.3.3
// PROJECT: SquareLine_Project

#include <string.h>
#include <stdio.h>

#include "log.h"
#include "ui_helpers.h"

#include "espnow.h"
#include "syslog.h"
#include "syscfg.h"
#include "sys_config.h"
#include "sys_status.h"
#include "hid_config.h"
#include "command.h"
#include "comm_packet.h"

static bool updated_main_flag = false;
static bool updated_child_flag = false;
static const char* TAG = "UI_EVENT";

void OnStartEvent(lv_event_t* e) {
  payload_t payload = { 0 };
  stage_t stage = NONE_STAGE;

  config_value_t* config = &payload.config;

  // Read config value
  if (read_hid_config(config)) {
    // Check if the configuration files are valid or not.
    // Start time should be included in irrigation available time.

    for (int i = 0; i < config->zone_cnt; i++) {
      LOGI(TAG, "zones = %d", config->zones[i]);
      LOGI(TAG, "flow = %d", config->flow_rate[i]);
    }
    show_timestamp(config->start_time);

    // validation check for start time : two time zone can be available for irrigation
    // device wakeup time : 5 am ~ 10 am (morning time), 5pm ~ 9pm (night time)
    // 05:00 ~ 10:00, 17:00 ~ 21:00 (0 ~ 24 hour)
    // device deep sleep time : 10:10 am ~ 04:50pm, 09:10 pm ~ 04:50 am
    // irrigation available time zones : 05:00 ~ 10:00 , 17:00 ~ 21:00
    if (!validation_of_start_irrigation_time(config->start_time, &stage)) {
      LOGW(TAG, "start_time is not available, please correct start time!!!");
      syscfg_unset(CFG_DATA, "hid_config");
      warnning_msgbox("start time is not available, please correct start time");
      return;
    }

    if (stage == CURR_STAGE || stage == NEXT_STAGE) {
      // Send configuration data to the main controller via ESP-NOW
      send_command_data(SET_CONFIG, NONE, &payload, sizeof(payload_t));
      LOGI(TAG, "Success to send Start command!!!");
      // Reset configuration data that is available in this current time,
      // if conf data will be available in next irrigation time, it should be keep and will send command in next time.
      syscfg_unset(CFG_DATA, "hid_config");
      // Set start irrigation flag
      set_start_irrigation(1);
    }
  } else {
    LOGE(TAG, "Failed to send Start command!!!");
    warnning_msgbox("There is no setting data");
  }
}

void OnStopEvent(lv_event_t* e) {
  send_command_data(FORCE_STOP, NONE, NULL, 0);
  set_stop_irrigation(1);
}

void OnSettingEvent(lv_event_t* e) {
  reset_settings();
}

void OnResetEvent(lv_event_t* e) {}

void OnFlowRateEvent(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    lv_keyboard_set_textarea(ui_SettingKeyboard, ui_FlowRateText);
  }
}

void OnTimeHourEvent(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    lv_keyboard_set_textarea(ui_SettingKeyboard, ui_TimeHourText);
  }
}

void OnTimeMinuteEvent(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    lv_keyboard_set_textarea(ui_SettingKeyboard, ui_TimeMinuteText);
  }
}

void OnSettingSaveEvent(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    const char* flow = lv_textarea_get_text(ui_FlowRateText);
    const char* zones = (const char*)get_checked_zones();
    const char* start_time_hour = lv_textarea_get_text(ui_TimeHourText);
    const char* start_time_minute = lv_textarea_get_text(ui_TimeMinuteText);
    if (save_hid_config(flow, start_time_hour, start_time_minute, zones)) {
      LOGI(TAG, "Success to save the configuration!!!");
    }
  }
}

void DeviceManagementEvent(char* type, lv_event_t* e) {
  int k = 0;
  char mac[16] = { 0 };
  char list_buf[200] = { 0 };

  LOGI(TAG, "type : %s", type);

  if (strncmp(type, "Master", 6) == 0) {
    lv_dropdown_set_selected(ui_DM_screen_Dropdown, 0);
    if (syscfg_get(SYSCFG_I_MASTER_MAC, SYSCFG_N_MASTER_MAC, mac, sizeof(mac)) == 0) {
      snprintf(list_buf, sizeof(mac) + 1, "%s\n", mac);
      lv_roller_set_options(ui_DM_Roller, list_buf, LV_ROLLER_MODE_NORMAL);
    }
  } else if (strncmp(type, "Child", 5) == 0) {
    lv_dropdown_set_selected(ui_DM_screen_Dropdown, 1);
    if (syscfg_get(SYSCFG_I_CHILD1_MAC, SYSCFG_N_CHILD1_MAC, mac, sizeof(mac)) == 0) {
      k += snprintf(list_buf + k, sizeof(mac) + 1, "%s\n", mac);
    }

    if (syscfg_get(SYSCFG_I_CHILD2_MAC, SYSCFG_N_CHILD2_MAC, mac, sizeof(mac)) == 0) {
      k += snprintf(list_buf + k, sizeof(mac) + 1, "%s\n", mac);
    }

    if (syscfg_get(SYSCFG_I_CHILD3_MAC, SYSCFG_N_CHILD3_MAC, mac, sizeof(mac)) == 0) {
      k += snprintf(list_buf + k, sizeof(mac) + 1, "%s\n", mac);
    }

    if (syscfg_get(SYSCFG_I_CHILD4_MAC, SYSCFG_N_CHILD4_MAC, mac, sizeof(mac)) == 0) {
      k += snprintf(list_buf + k, sizeof(mac) + 1, "%s\n", mac);
    }

    if (syscfg_get(SYSCFG_I_CHILD5_MAC, SYSCFG_N_CHILD5_MAC, mac, sizeof(mac)) == 0) {
      k += snprintf(list_buf + k, sizeof(mac) + 1, "%s\n", mac);
    }

    if (syscfg_get(SYSCFG_I_CHILD6_MAC, SYSCFG_N_CHILD6_MAC, mac, sizeof(mac)) == 0) {
      k += snprintf(list_buf + k, sizeof(mac) + 1, "%s\n", mac);
    }

    // remove last lf
    list_buf[k - 1] = '\0';

    lv_roller_set_options(ui_DM_Roller, list_buf, LV_ROLLER_MODE_NORMAL);

  } else {
    LOGI(TAG, "Error!!");
  }
}

void DM_roller_event(char* mac_addr, lv_event_t* e) {
  char mac[16] = { 0 };
  LOGI(TAG, "selected mac address : %s", mac_addr);

  if (syscfg_get(SYSCFG_I_MASTER_MAC, SYSCFG_N_MASTER_MAC, mac, sizeof(mac)) == 0) {
    LOGI(TAG, "mac_addr = %s, mac = %s", mac_addr, mac);
    if (strncmp(mac_addr, mac, strlen(mac)) == 0) {
      lv_label_set_text(ui_DM_Roller_Label, "Master");
      return;
    }
  }

  if (syscfg_get(SYSCFG_I_CHILD1_MAC, SYSCFG_N_CHILD1_MAC, mac, sizeof(mac)) == 0) {
    LOGI(TAG, "mac_addr = %s, mac = %s", mac_addr, mac);
    if (strncmp(mac_addr, mac, strlen(mac)) == 0) {
      lv_label_set_text(ui_DM_Roller_Label, "Child_1");
      return;
    }
  }

  if (syscfg_get(SYSCFG_I_CHILD2_MAC, SYSCFG_N_CHILD2_MAC, mac, sizeof(mac)) == 0) {
    LOGI(TAG, "mac_addr = %s, mac = %s", mac_addr, mac);
    if (strncmp(mac_addr, mac, strlen(mac)) == 0) {
      lv_label_set_text(ui_DM_Roller_Label, "Child_2");
      return;
    }
  }

  if (syscfg_get(SYSCFG_I_CHILD3_MAC, SYSCFG_N_CHILD3_MAC, mac, sizeof(mac)) == 0) {
    LOGI(TAG, "mac_addr = %s, mac = %s", mac_addr, mac);
    if (strncmp(mac_addr, mac, strlen(mac)) == 0) {
      lv_label_set_text(ui_DM_Roller_Label, "Child_3");
      return;
    }
  }

  if (syscfg_get(SYSCFG_I_CHILD4_MAC, SYSCFG_N_CHILD4_MAC, mac, sizeof(mac)) == 0) {
    LOGI(TAG, "mac_addr = %s, mac = %s", mac_addr, mac);
    if (strncmp(mac_addr, mac, strlen(mac)) == 0) {
      lv_label_set_text(ui_DM_Roller_Label, "Child_4");
      return;
    }
  }

  if (syscfg_get(SYSCFG_I_CHILD5_MAC, SYSCFG_N_CHILD5_MAC, mac, sizeof(mac)) == 0) {
    LOGI(TAG, "mac_addr = %s, mac = %s", mac_addr, mac);
    if (strncmp(mac_addr, mac, strlen(mac)) == 0) {
      lv_label_set_text(ui_DM_Roller_Label, "Child_5");
      return;
    }
  }

  if (syscfg_get(SYSCFG_I_CHILD6_MAC, SYSCFG_N_CHILD6_MAC, mac, sizeof(mac)) == 0) {
    LOGI(TAG, "mac_addr = %s, mac = %s", mac_addr, mac);
    if (strncmp(mac_addr, mac, strlen(mac)) == 0) {
      lv_label_set_text(ui_DM_Roller_Label, "Child_6");
      return;
    }
  }
}

void update_mac_address(void) {
  char* cfg_name[7] = { SYSCFG_N_MASTER_MAC, SYSCFG_N_CHILD1_MAC, SYSCFG_N_CHILD2_MAC, SYSCFG_N_CHILD3_MAC,
                        SYSCFG_N_CHILD4_MAC, SYSCFG_N_CHILD5_MAC, SYSCFG_N_CHILD6_MAC };
  device_type_t dev_type[7] = { MAIN_DEV, CHILD_1, CHILD_2, CHILD_3, CHILD_4, CHILD_5, CHILD_6 };
  int dev_cnt = 0;
  char mac_addr[13];

  payload_t payload = { 0 };
  device_manage_t* dev_manage = (device_manage_t*)&payload.dev_manage;

  for (int i = 0; i < 7; i++) {
    memset(mac_addr, 0x00, sizeof(mac_addr));
    if (syscfg_get(MFG_DATA, cfg_name[i], mac_addr, sizeof(mac_addr)) == 0) {
      memcpy(&dev_manage->update_dev_addr[dev_cnt].mac_addr, mac_addr, sizeof(mac_addr));
      dev_manage->update_dev_addr[dev_cnt].device_type = dev_type[i];
      dev_cnt++;
    }
  }

  if (updated_main_flag && (dev_manage->update_dev_addr[0].device_type == MAIN_DEV)) {
    syscfg_set(MFG_DATA, cfg_name[0], dev_manage->update_dev_addr[0].mac_addr);
    espnow_add_peers(HID_DEVICE);
    updated_main_flag = false;
  }

  if (updated_child_flag) {
    dev_manage->update_dev_cnt = dev_cnt;
    send_command_data(UPDATE_DEVICE_ADDR, NONE, &payload, sizeof(payload_t));
    updated_child_flag = false;
  }
}

void delete_mac_address(const char* device_type, char* mac_addr) {
  char mac[16] = { 0 };
  uint8_t dev_addr[MAC_ADDR_LEN] = { 0 };

  if (strncmp(device_type, "Master", strlen("Master")) == 0) {
    syscfg_get(SYSCFG_I_MASTER_MAC, SYSCFG_N_MASTER_MAC, mac, sizeof(mac));
    if (strncmp(mac, mac_addr, strlen(mac)) == 0) {
      syscfg_unset(SYSCFG_I_MASTER_MAC, SYSCFG_N_MASTER_MAC);
      ascii_to_hex(mac, MAC_ADDR_LEN * 2, dev_addr);
      espnow_remove_peer(dev_addr);
      updated_main_flag = true;
    }
    return;
  }

  if (strncmp(device_type, "Child_1", strlen("Child_1")) == 0) {
    syscfg_get(SYSCFG_I_CHILD1_MAC, SYSCFG_N_CHILD1_MAC, mac, sizeof(mac));
    if (strncmp(mac, mac_addr, strlen(mac)) == 0) {
      syscfg_unset(SYSCFG_I_CHILD1_MAC, SYSCFG_N_CHILD1_MAC);
      updated_child_flag = true;
    }
    return;
  }

  if (strncmp(device_type, "Child_2", strlen("Child_2")) == 0) {
    syscfg_get(SYSCFG_I_CHILD2_MAC, SYSCFG_N_CHILD2_MAC, mac, sizeof(mac));
    if (strncmp(mac, mac_addr, strlen(mac)) == 0) {
      syscfg_unset(SYSCFG_I_CHILD2_MAC, SYSCFG_N_CHILD2_MAC);
      updated_child_flag = true;
    }
    return;
  }

  if (strncmp(device_type, "Child_3", strlen("Child_3")) == 0) {
    syscfg_get(SYSCFG_I_CHILD3_MAC, SYSCFG_N_CHILD3_MAC, mac, sizeof(mac));
    if (strncmp(mac, mac_addr, strlen(mac)) == 0) {
      syscfg_unset(SYSCFG_I_CHILD3_MAC, SYSCFG_N_CHILD3_MAC);
      updated_child_flag = true;
    }
    return;
  }

  if (strncmp(device_type, "Child_4", strlen("Child_4")) == 0) {
    syscfg_get(SYSCFG_I_CHILD4_MAC, SYSCFG_N_CHILD4_MAC, mac, sizeof(mac));
    if (strncmp(mac, mac_addr, strlen(mac)) == 0) {
      syscfg_unset(SYSCFG_I_CHILD4_MAC, SYSCFG_N_CHILD4_MAC);
      updated_child_flag = true;
    }
    return;
  }

  if (strncmp(device_type, "Child_5", strlen("Child_5")) == 0) {
    syscfg_get(SYSCFG_I_CHILD5_MAC, SYSCFG_N_CHILD5_MAC, mac, sizeof(mac));
    if (strncmp(mac, mac_addr, strlen(mac)) == 0) {
      syscfg_unset(SYSCFG_I_CHILD5_MAC, SYSCFG_N_CHILD5_MAC);
      updated_child_flag = true;
    }
    return;
  }

  if (strncmp(device_type, "Child_6", strlen("Child_6")) == 0) {
    syscfg_get(SYSCFG_I_CHILD6_MAC, SYSCFG_N_CHILD6_MAC, mac, sizeof(mac));
    if (strncmp(mac, mac_addr, strlen(mac)) == 0) {
      syscfg_unset(SYSCFG_I_CHILD6_MAC, SYSCFG_N_CHILD6_MAC);
      updated_child_flag = true;
    }
    return;
  }
}
