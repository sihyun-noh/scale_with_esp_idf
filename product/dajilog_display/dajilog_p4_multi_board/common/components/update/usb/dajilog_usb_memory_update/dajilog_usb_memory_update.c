#include "dajilog_usb_memory_update.h"

#include <stdio.h>
#include <string.h>

#include "esp_app_desc.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_msc_host.h"
#include "esp_msc_ota.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define UPDATE_FILE_PATH "/usb/DAJILOG_UPDATE.bin"
#define UPDATE_TASK_STACK_SIZE 6144
#define UPDATE_BUFFER_SIZE 4096

static const char *TAG = "dajilog_usb_drive";
static portMUX_TYPE s_lock = portMUX_INITIALIZER_UNLOCKED;
static esp_msc_host_handle_t s_host;
static bool s_started;
static bool s_starting;
static bool s_apply_running;
static dajilog_usb_memory_update_status_t s_status = {
    .state = DAJILOG_USB_MEMORY_IDLE,
    .message = "USB drive updater is not initialized",
};

/** @brief Replace the mutex-protected removable-drive update status. */
static void set_status(dajilog_usb_memory_update_state_t state, esp_err_t error,
                       int progress, const char *message)
{
    portENTER_CRITICAL(&s_lock);
    s_status.state = state;
    s_status.last_error = error;
    s_status.progress_percent = progress;
    strlcpy(s_status.message, message, sizeof(s_status.message));
    portEXIT_CRITICAL(&s_lock);
}

/** @brief Enter the update error state with a user-facing message. */
static void set_error(esp_err_t error, const char *message)
{
    char text[sizeof(s_status.message)];
    snprintf(text, sizeof(text), "%s (%s)", message, esp_err_to_name(error));
    ESP_LOGE(TAG, "%s", text);
    set_status(DAJILOG_USB_MEMORY_ERROR, error, 0, text);
}

void dajilog_usb_memory_update_get_status(dajilog_usb_memory_update_status_t *status)
{
    if (status == NULL) {
        return;
    }
    portENTER_CRITICAL(&s_lock);
    *status = s_status;
    portEXIT_CRITICAL(&s_lock);
}

/** @brief Track USB Host MSC device connection and disconnection. */
static void host_event_callback(esp_msc_host_handle_t handle,
                                esp_msc_host_event_t event, void *user_ctx)
{
    (void)handle;
    (void)user_ctx;
    switch (event) {
    case ESP_MSC_HOST_CONNECT:
        ESP_LOGI(TAG, "USB MSC device connected");
        set_status(DAJILOG_USB_MEMORY_WAITING, ESP_OK, 0, "USB drive detected. Mounting...");
        break;
    case ESP_MSC_HOST_VFS_REGISTER:
        ESP_LOGI(TAG, "USB MSC FAT filesystem mounted at /usb");
        set_status(DAJILOG_USB_MEMORY_MOUNTED, ESP_OK, 0, "USB drive ready. Tap Update");
        break;
    case ESP_MSC_HOST_DISCONNECT:
        ESP_LOGW(TAG, "USB MSC device disconnected");
        if (!s_apply_running) {
            set_status(DAJILOG_USB_MEMORY_WAITING, ESP_OK, 0,
                       "Insert a FAT USB drive into the OTG port");
        }
        break;
    case ESP_MSC_HOST_VFS_UNREGISTER:
        ESP_LOGI(TAG, "USB MSC FAT filesystem unmounted");
        if (!s_apply_running) {
            set_status(DAJILOG_USB_MEMORY_WAITING, ESP_OK, 0,
                       "Insert a FAT USB drive into the OTG port");
        }
        break;
    default:
        break;
    }
}

/** @brief Convert MSC OTA progress events into a percentage for the UI. */
static void ota_event_handler(void *arg, esp_event_base_t base,
                              int32_t event_id, void *event_data)
{
    (void)arg;
    (void)base;
    if (event_id == ESP_MSC_OTA_WRITE_FLASH && event_data != NULL) {
        int percent = (int)(*(float *)event_data * 100.0f);
        set_status(DAJILOG_USB_MEMORY_WRITING, ESP_OK, percent,
                   "Installing firmware. Do not remove the USB drive");
    }
}

/** @brief Reject firmware built for another chip or product identity. */
static esp_err_t validate_product(esp_msc_ota_handle_t ota)
{
    esp_app_desc_t candidate;
    esp_err_t err = esp_msc_ota_get_img_desc(ota, &candidate);
    if (err != ESP_OK) {
        return err;
    }

    const esp_app_desc_t *running = esp_app_get_description();
    if (running == NULL || strncmp(candidate.project_name, running->project_name,
                                   sizeof(candidate.project_name)) != 0) {
        ESP_LOGE(TAG, "Firmware product mismatch");
        return ESP_ERR_INVALID_VERSION;
    }
    if (memcmp(candidate.app_elf_sha256, running->app_elf_sha256,
               sizeof(candidate.app_elf_sha256)) == 0) {
        ESP_LOGW(TAG, "Firmware image is identical to the running firmware");
        return ESP_ERR_INVALID_VERSION;
    }
    ESP_LOGI(TAG, "USB drive firmware accepted: version %s", candidate.version);
    return ESP_OK;
}

/** @brief Validate and install removable-drive firmware outside the UI task. */
static void apply_task(void *arg)
{
    (void)arg;
    esp_msc_ota_handle_t ota = NULL;
    const esp_msc_ota_config_t config = {
        .host_handle = s_host,
        .ota_bin_path = UPDATE_FILE_PATH,
        .wait_msc_connect = pdMS_TO_TICKS(1000),
        .buffer_size = UPDATE_BUFFER_SIZE,
        .bulk_flash_erase = false,
    };

    set_status(DAJILOG_USB_MEMORY_VALIDATING, ESP_OK, 0,
               "Validating DAJILOG_UPDATE.bin...");
    esp_err_t err = esp_msc_ota_begin(&config, &ota);
    if (err == ESP_OK) {
        err = validate_product(ota);
    }
    while (err == ESP_OK && !esp_msc_ota_is_complete_data_received(ota)) {
        err = esp_msc_ota_perform(ota);
    }
    if (err == ESP_OK) {
        err = esp_msc_ota_end(ota);
        ota = NULL;
    } else if (ota != NULL) {
        esp_msc_ota_abort(ota);
        ota = NULL;
    }

    if (err == ESP_OK) {
        set_status(DAJILOG_USB_MEMORY_READY_TO_REBOOT, ESP_OK, 100,
                   "Update complete. Remove USB drive and tap Restart");
        ESP_LOGI(TAG, "USB drive firmware update completed");
    } else {
        set_error(err, "USB drive firmware update failed");
    }
    s_apply_running = false;
    vTaskDelete(NULL);
}

esp_err_t dajilog_usb_memory_update_start(void)
{
    portENTER_CRITICAL(&s_lock);
    if (s_started || s_starting) {
        portEXIT_CRITICAL(&s_lock);
        return ESP_OK;
    }
    s_starting = true;
    portEXIT_CRITICAL(&s_lock);

    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        goto fail;
    }
    err = esp_event_handler_register(ESP_MSC_OTA_EVENT, ESP_EVENT_ANY_ID,
                                     ota_event_handler, NULL);
    if (err != ESP_OK) {
        goto fail;
    }

    esp_msc_host_config_t config = {
        .base_path = "/usb",
        .host_config = {
            .skip_phy_setup = false,
            .root_port_unpowered = false,
            .intr_flags = ESP_INTR_FLAG_LEVEL1,
            .peripheral_map = 1U << 0, /* ESP32-P4 High-Speed USB OTG */
            .fsls_only = false,
        },
        .host_driver_config = DEFAULT_MSC_HOST_DRIVER_CONFIG(),
        .vfs_fat_mount_config = DEFAULT_ESP_VFS_FAT_MOUNT_CONFIG(),
        .skip_init_usb_host_driver = false,
        .event_cb = host_event_callback,
        .event_cb_arg = NULL,
    };
    err = esp_msc_host_install(&config, &s_host);
    if (err != ESP_OK) {
        esp_event_handler_unregister(ESP_MSC_OTA_EVENT, ESP_EVENT_ANY_ID,
                                     ota_event_handler);
        goto fail;
    }

    portENTER_CRITICAL(&s_lock);
    s_started = true;
    s_starting = false;
    portEXIT_CRITICAL(&s_lock);
    set_status(DAJILOG_USB_MEMORY_WAITING, ESP_OK, 0,
               "Insert a FAT USB drive into the OTG port");
    ESP_LOGI(TAG, "USB flash-drive update service started on ESP32-P4 HS OTG port");
    return ESP_OK;

fail:
    portENTER_CRITICAL(&s_lock);
    s_starting = false;
    portEXIT_CRITICAL(&s_lock);
    set_error(err, "USB Host start failed; reboot if PC updater was used");
    return err;
}

esp_err_t dajilog_usb_memory_update_apply(void)
{
    if (!s_started || s_apply_running || !esp_msc_host_is_mounted(s_host)) {
        return ESP_ERR_INVALID_STATE;
    }
    s_apply_running = true;
    if (xTaskCreate(apply_task, "usb_drive_ota", UPDATE_TASK_STACK_SIZE,
                    NULL, 5, NULL) != pdPASS) {
        s_apply_running = false;
        set_error(ESP_ERR_NO_MEM, "USB drive OTA task creation failed");
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

esp_err_t dajilog_usb_memory_update_reboot(void)
{
    dajilog_usb_memory_update_status_t status;
    dajilog_usb_memory_update_get_status(&status);
    if (status.state != DAJILOG_USB_MEMORY_READY_TO_REBOOT) {
        return ESP_ERR_INVALID_STATE;
    }
    esp_restart();
    return ESP_OK;
}
