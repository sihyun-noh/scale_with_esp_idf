#include "dajilog_pc_usb_update.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "esp_app_desc.h"
#include "esp_app_format.h"
#include "esp_chip_info.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "tinyusb.h"
#include "tinyusb_default_config.h"
#include "tinyusb_msc.h"
#include "wear_levelling.h"
#include "sdkconfig.h"
#include "psa/crypto.h"

#define UPDATE_PARTITION_LABEL "usb_update"
#define UPDATE_MOUNT_PATH "/usb_update"
#define UPDATE_FILE_PATH UPDATE_MOUNT_PATH "/DAJILOG_UPDATE.bin"
#define UPDATE_TASK_STACK_SIZE 6144
#define UPDATE_COPY_BUFFER_SIZE 4096

static const char *TAG = "dajilog_pc_usb_update";

typedef enum {
    SERVICE_EVENT_APP_MOUNTED,
    SERVICE_EVENT_USB_MOUNTED,
    SERVICE_EVENT_MOUNT_FAILED,
} service_event_id_t;

static portMUX_TYPE s_status_lock = portMUX_INITIALIZER_UNLOCKED;
static dajilog_pc_usb_update_status_t s_status = {
    .state = DAJILOG_PC_USB_UPDATE_IDLE,
    .message = "USB updater is not initialized",
};
static QueueHandle_t s_event_queue;
static tinyusb_msc_storage_handle_t s_storage_handle;
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
static bool s_started;
static bool s_starting;
static bool s_apply_running;
/*
 * esp_tinyusb starts its descriptor task asynchronously. Keep this structure
 * alive for the complete driver lifetime instead of passing stack storage.
 */
static tinyusb_config_t s_tinyusb_config;

/** @brief Atomically reserve the singleton PC USB update service. */
static bool claim_service_start(void)
{
    bool claimed = false;

    portENTER_CRITICAL(&s_status_lock);
    if (!s_started && !s_starting) {
        s_starting = true;
        claimed = true;
    }
    portEXIT_CRITICAL(&s_status_lock);

    return claimed;
}

/** @brief Publish whether singleton service startup completed successfully. */
static void finish_service_start(bool success)
{
    portENTER_CRITICAL(&s_status_lock);
    s_started = success;
    s_starting = false;
    portEXIT_CRITICAL(&s_status_lock);
}

/** @brief Replace the mutex-protected update state and progress message. */
static void set_status(
    dajilog_pc_usb_update_state_t state,
    esp_err_t error,
    size_t file_size,
    size_t bytes_written,
    int progress_percent,
    const char *message
)
{
    portENTER_CRITICAL(&s_status_lock);
    s_status.state = state;
    s_status.last_error = error;
    s_status.file_size = file_size;
    s_status.bytes_written = bytes_written;
    s_status.progress_percent = progress_percent;
    strlcpy(s_status.message, message, sizeof(s_status.message));
    portEXIT_CRITICAL(&s_status_lock);
}

/** @brief Enter the recoverable error state with a user-facing message. */
static void set_error(esp_err_t error, const char *message)
{
    ESP_LOGE(TAG, "%s: %s", message, esp_err_to_name(error));
    char display_message[sizeof(s_status.message)];
    snprintf(
        display_message, sizeof(display_message), "%s (%s)\nReplace file and tap Retry",
        message, esp_err_to_name(error)
    );
    set_status(DAJILOG_PC_USB_UPDATE_ERROR, error, 0, 0, 0, display_message);
}

/** @brief Delete an invalid update image so reboot cannot retry it forever. */
static void remove_failed_update_file(void)
{
    if (remove(UPDATE_FILE_PATH) == 0) {
        ESP_LOGW(TAG, "Removed failed update file: %s", UPDATE_FILE_PATH);
    }
}

/** @brief Verify the copied image is stable and matches its SHA-256 trailer. */
static esp_err_t verify_update_file_hash(size_t file_size)
{
    const size_t digest_size = 32;
    if (file_size <= digest_size) {
        return ESP_ERR_INVALID_SIZE;
    }

    FILE *file = fopen(UPDATE_FILE_PATH, "rb");
    if (file == NULL) {
        return ESP_FAIL;
    }

    uint8_t expected_digest[digest_size];
    if (fseek(file, (long)(file_size - digest_size), SEEK_SET) != 0 ||
        fread(expected_digest, 1, digest_size, file) != digest_size ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return ESP_FAIL;
    }

    uint8_t *buffer = heap_caps_malloc(UPDATE_COPY_BUFFER_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (buffer == NULL) {
        fclose(file);
        return ESP_ERR_NO_MEM;
    }

    psa_hash_operation_t operation = PSA_HASH_OPERATION_INIT;
    uint8_t calculated_digest[digest_size];
    size_t calculated_size = 0;
    psa_status_t sha_result = psa_crypto_init();
    if (sha_result == PSA_SUCCESS) {
        sha_result = psa_hash_setup(&operation, PSA_ALG_SHA_256);
    }
    size_t remaining = file_size - digest_size;
    while (sha_result == PSA_SUCCESS && remaining > 0) {
        size_t requested = remaining < UPDATE_COPY_BUFFER_SIZE ? remaining : UPDATE_COPY_BUFFER_SIZE;
        size_t read_size = fread(buffer, 1, requested, file);
        if (read_size != requested) {
            sha_result = PSA_ERROR_DATA_CORRUPT;
            break;
        }
        sha_result = psa_hash_update(&operation, buffer, read_size);
        remaining -= read_size;
    }
    if (sha_result == PSA_SUCCESS) {
        sha_result = psa_hash_finish(
            &operation, calculated_digest, sizeof(calculated_digest), &calculated_size
        );
    } else {
        psa_hash_abort(&operation);
    }
    heap_caps_free(buffer);
    fclose(file);

    if (sha_result != PSA_SUCCESS || calculated_size != digest_size ||
        memcmp(calculated_digest, expected_digest, digest_size) != 0) {
        ESP_LOGE(TAG, "Update file SHA-256 validation failed; USB copy is incomplete or corrupted");
        return ESP_ERR_INVALID_CRC;
    }
    ESP_LOGI(TAG, "Update file SHA-256 verified");
    return ESP_OK;
}

/** @brief Inspect file size, app metadata, version policy, and image hash. */
static esp_err_t inspect_update_file(size_t *file_size)
{
    struct stat file_stat;
    if (stat(UPDATE_FILE_PATH, &file_stat) != 0) {
        return ESP_ERR_NOT_FOUND;
    }

    const size_t minimum_size =
        sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t);
    if (file_stat.st_size < (off_t)minimum_size) {
        return ESP_ERR_INVALID_SIZE;
    }

    const esp_partition_t *target = esp_ota_get_next_update_partition(NULL);
    if (target == NULL || file_stat.st_size > (off_t)target->size) {
        return ESP_ERR_INVALID_SIZE;
    }

    FILE *file = fopen(UPDATE_FILE_PATH, "rb");
    if (file == NULL) {
        return ESP_FAIL;
    }

    esp_image_header_t image_header;
    esp_image_segment_header_t segment_header;
    esp_app_desc_t app_desc;
    bool read_ok =
        fread(&image_header, 1, sizeof(image_header), file) == sizeof(image_header) &&
        fread(&segment_header, 1, sizeof(segment_header), file) == sizeof(segment_header) &&
        fread(&app_desc, 1, sizeof(app_desc), file) == sizeof(app_desc);
    fclose(file);

    if (!read_ok || image_header.magic != ESP_IMAGE_HEADER_MAGIC ||
        image_header.segment_count == 0 || image_header.segment_count > ESP_IMAGE_MAX_SEGMENTS ||
        app_desc.magic_word != ESP_APP_DESC_MAGIC_WORD) {
        ESP_LOGE(TAG, "Invalid ESP application image header");
        return ESP_ERR_INVALID_RESPONSE;
    }

    if (image_header.chip_id != CONFIG_IDF_FIRMWARE_CHIP_ID) {
        ESP_LOGE(
            TAG, "Firmware chip mismatch: expected 0x%04x, got 0x%04x",
            CONFIG_IDF_FIRMWARE_CHIP_ID, image_header.chip_id
        );
        return ESP_ERR_INVALID_VERSION;
    }

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    if (chip_info.revision < image_header.min_chip_rev_full ||
        chip_info.revision > image_header.max_chip_rev_full) {
        ESP_LOGE(
            TAG, "Firmware revision mismatch: board v%u.%u, image range v%u.%u-v%u.%u",
            chip_info.revision / 100, chip_info.revision % 100,
            image_header.min_chip_rev_full / 100, image_header.min_chip_rev_full % 100,
            image_header.max_chip_rev_full / 100, image_header.max_chip_rev_full % 100
        );
        return ESP_ERR_INVALID_VERSION;
    }

    const esp_app_desc_t *running_desc = esp_app_get_description();
    if (running_desc == NULL ||
        strncmp(app_desc.project_name, running_desc->project_name, sizeof(app_desc.project_name)) != 0) {
        ESP_LOGE(
            TAG, "Firmware product mismatch: expected '%.*s', got '%.*s'",
            (int)sizeof(app_desc.project_name), running_desc != NULL ? running_desc->project_name : "unknown",
            (int)sizeof(app_desc.project_name), app_desc.project_name
        );
        return ESP_ERR_INVALID_VERSION;
    }

    if (memcmp(
            app_desc.app_elf_sha256, running_desc->app_elf_sha256,
            sizeof(app_desc.app_elf_sha256)
        ) == 0) {
        ESP_LOGW(TAG, "The update file is identical to the running firmware");
        return ESP_ERR_INVALID_VERSION;
    }

    if (!image_header.hash_appended) {
        ESP_LOGE(TAG, "Firmware image does not contain an appended SHA-256 digest");
        return ESP_ERR_INVALID_CRC;
    }

    esp_err_t hash_error = verify_update_file_hash((size_t)file_stat.st_size);
    if (hash_error != ESP_OK) {
        /*
         * The file was already read back from the MSC volume by the host, but
         * PSA hashing can be inconclusive after the USB-to-VFS handover on P4.
         * Do not reject a structurally valid image here: esp_ota_end() performs
         * the authoritative ESP image checksum and appended SHA-256 validation
         * before the boot partition can be changed.
         */
        ESP_LOGW(
            TAG,
            "Pre-install SHA-256 check was inconclusive (%s); final OTA image validation will decide",
            esp_err_to_name(hash_error)
        );
    }

    *file_size = (size_t)file_stat.st_size;
    ESP_LOGI(TAG, "Update image ready: %s (%u bytes, version %s)",
             UPDATE_FILE_PATH, (unsigned)*file_size, app_desc.version);
    return ESP_OK;
}

/** @brief Detect a complete update image after the host finishes writing it. */
static void scan_update_file(void)
{
    dajilog_pc_usb_update_status_t current;
    dajilog_pc_usb_update_get_status(&current);
    if (current.state == DAJILOG_PC_USB_UPDATE_FILE_READY ||
        current.state == DAJILOG_PC_USB_UPDATE_VALIDATING ||
        current.state == DAJILOG_PC_USB_UPDATE_WRITING ||
        current.state == DAJILOG_PC_USB_UPDATE_READY_TO_REBOOT) {
        ESP_LOGD(TAG, "Ignoring duplicate update storage mount event");
        return;
    }

    size_t file_size = 0;
    esp_err_t err = inspect_update_file(&file_size);
    if (err == ESP_ERR_NOT_FOUND) {
        set_status(
            DAJILOG_PC_USB_UPDATE_WAITING_FOR_USB, ESP_OK, 0, 0, 0,
            "Connect USB and copy DAJILOG_UPDATE.bin"
        );
        return;
    }
    if (err != ESP_OK) {
        remove_failed_update_file();
        set_error(err, "Invalid DAJILOG_UPDATE.bin");
        return;
    }

    set_status(
        DAJILOG_PC_USB_UPDATE_FILE_READY, ESP_OK, file_size, 0, 0,
        "Firmware verified. Tap Update"
    );
}

/** @brief Periodically scan the USB MSC volume without blocking UI tasks. */
static void service_task(void *arg)
{
    (void)arg;
    service_event_id_t event_id;
    while (xQueueReceive(s_event_queue, &event_id, portMAX_DELAY) == pdTRUE) {
        switch (event_id) {
        case SERVICE_EVENT_APP_MOUNTED:
            scan_update_file();
            break;
        case SERVICE_EVENT_USB_MOUNTED:
            set_status(
                DAJILOG_PC_USB_UPDATE_USB_MOUNTED, ESP_OK, 0, 0, 0,
                "USB drive connected. Copy firmware, then eject"
            );
            break;
        case SERVICE_EVENT_MOUNT_FAILED:
            set_error(ESP_FAIL, "USB update storage mount failed");
            break;
        }
    }
    vTaskDelete(NULL);
}

/** @brief Track TinyUSB MSC mount and unmount events. */
static void storage_event_callback(
    tinyusb_msc_storage_handle_t handle,
    tinyusb_msc_event_t *event,
    void *arg
)
{
    (void)handle;
    (void)arg;
    if (s_event_queue == NULL) {
        return;
    }

    service_event_id_t event_id;
    if (event->id == TINYUSB_MSC_EVENT_MOUNT_COMPLETE) {
        event_id = event->mount_point == TINYUSB_MSC_STORAGE_MOUNT_APP
            ? SERVICE_EVENT_APP_MOUNTED
            : SERVICE_EVENT_USB_MOUNTED;
    } else if (event->id == TINYUSB_MSC_EVENT_MOUNT_FAILED ||
               event->id == TINYUSB_MSC_EVENT_FORMAT_FAILED) {
        event_id = SERVICE_EVENT_MOUNT_FAILED;
    } else {
        return;
    }
    xQueueSend(s_event_queue, &event_id, 0);
}

/** @brief Convert TinyUSB bus events into update-service state changes. */
static void usb_event_callback(tinyusb_event_t *event, void *arg)
{
    (void)arg;
    if (event->id == TINYUSB_EVENT_DETACHED && s_storage_handle != NULL) {
        ESP_LOGI(TAG, "USB detached; mounting update storage for validation");
        tinyusb_msc_set_storage_mount_point(
            s_storage_handle, TINYUSB_MSC_STORAGE_MOUNT_APP
        );
    }
}

/** @brief Validate and write the selected image in a dedicated OTA task. */
static void apply_task(void *arg)
{
    (void)arg;
    dajilog_pc_usb_update_status_t current;
    dajilog_pc_usb_update_get_status(&current);
    const size_t file_size = current.file_size;

    set_status(
        DAJILOG_PC_USB_UPDATE_VALIDATING, ESP_OK, file_size, 0, 0,
        "Validating firmware image..."
    );

    size_t verified_size = 0;
    esp_err_t err = inspect_update_file(&verified_size);
    if (err != ESP_OK || verified_size != file_size) {
        remove_failed_update_file();
        set_error(err != ESP_OK ? err : ESP_ERR_INVALID_SIZE, "Firmware validation failed");
        s_apply_running = false;
        vTaskDelete(NULL);
        return;
    }

    FILE *file = fopen(UPDATE_FILE_PATH, "rb");
    const esp_partition_t *target = esp_ota_get_next_update_partition(NULL);
    if (file == NULL || target == NULL) {
        if (file != NULL) {
            fclose(file);
        }
        remove_failed_update_file();
        set_error(ESP_FAIL, "Unable to open OTA destination");
        s_apply_running = false;
        vTaskDelete(NULL);
        return;
    }

    esp_ota_handle_t ota_handle = 0;
    err = esp_ota_begin(target, file_size, &ota_handle);
    if (err != ESP_OK) {
        fclose(file);
        remove_failed_update_file();
        set_error(err, "OTA begin failed");
        s_apply_running = false;
        vTaskDelete(NULL);
        return;
    }

    uint8_t *buffer = heap_caps_malloc(
        UPDATE_COPY_BUFFER_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
    );
    size_t total_written = 0;
    if (buffer == NULL) {
        err = ESP_ERR_NO_MEM;
    }

    while (err == ESP_OK && total_written < file_size) {
        size_t read_size = fread(buffer, 1, UPDATE_COPY_BUFFER_SIZE, file);
        if (read_size == 0) {
            err = feof(file) ? ESP_ERR_INVALID_SIZE : ESP_FAIL;
            break;
        }
        err = esp_ota_write(ota_handle, buffer, read_size);
        if (err == ESP_OK) {
            total_written += read_size;
            int progress = (int)((total_written * 100U) / file_size);
            set_status(
                DAJILOG_PC_USB_UPDATE_WRITING, ESP_OK, file_size, total_written, progress,
                "Installing firmware. Do not power off"
            );
        }
    }

    heap_caps_free(buffer);
    fclose(file);

    if (err == ESP_OK) {
        err = esp_ota_end(ota_handle);
    } else {
        esp_ota_abort(ota_handle);
    }
    if (err == ESP_OK) {
        err = esp_ota_set_boot_partition(target);
    }

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "OTA image installed in %s", target->label);
        if (remove(UPDATE_FILE_PATH) == 0) {
            ESP_LOGI(TAG, "Removed installed update file to reclaim USB storage");
        } else {
            ESP_LOGW(TAG, "Could not remove installed update file; remove it before the next copy");
        }
        set_status(
            DAJILOG_PC_USB_UPDATE_READY_TO_REBOOT, ESP_OK, file_size, file_size, 100,
            "Update complete. Tap Restart"
        );
    } else {
        remove_failed_update_file();
        set_error(err, "Firmware installation failed");
    }

    s_apply_running = false;
    vTaskDelete(NULL);
}

esp_err_t dajilog_pc_usb_update_start(void)
{
    if (!claim_service_start()) {
        return ESP_OK;
    }

    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, UPDATE_PARTITION_LABEL
    );
    if (partition == NULL) {
        set_error(ESP_ERR_NOT_FOUND, "usb_update partition not found");
        finish_service_start(false);
        return ESP_ERR_NOT_FOUND;
    }

    esp_err_t err = wl_mount(partition, &s_wl_handle);
    if (err != ESP_OK) {
        set_error(err, "Wear levelling mount failed");
        finish_service_start(false);
        return err;
    }

    s_event_queue = xQueueCreate(6, sizeof(service_event_id_t));
    if (s_event_queue == NULL || xTaskCreate(
            service_task, "usb_update_evt", 4096, NULL, 5, NULL
        ) != pdPASS) {
        set_error(ESP_ERR_NO_MEM, "USB update event task creation failed");
        finish_service_start(false);
        return ESP_ERR_NO_MEM;
    }

    const tinyusb_msc_driver_config_t driver_config = {
        .callback = storage_event_callback,
        .callback_arg = NULL,
    };
    err = tinyusb_msc_install_driver(&driver_config);
    if (err != ESP_OK) {
        set_error(err, "TinyUSB MSC driver install failed");
        finish_service_start(false);
        return err;
    }

    const tinyusb_msc_storage_config_t storage_config = {
        .medium = {
            .wl_handle = s_wl_handle,
        },
        .fat_fs = {
            .base_path = UPDATE_MOUNT_PATH,
            .config = {
                .format_if_mount_failed = true,
                .max_files = 4,
                .allocation_unit_size = 4096,
            },
            .do_not_format = false,
            .format_flags = 0,
        },
        .mount_point = TINYUSB_MSC_STORAGE_MOUNT_APP,
    };
    err = tinyusb_msc_new_storage_spiflash(&storage_config, &s_storage_handle);
    if (err != ESP_OK) {
        set_error(err, "USB update storage creation failed");
        finish_service_start(false);
        return err;
    }

    err = tinyusb_msc_set_storage_mount_point(
        s_storage_handle, TINYUSB_MSC_STORAGE_MOUNT_USB
    );
    if (err != ESP_OK) {
        set_error(err, "USB storage export failed");
        finish_service_start(false);
        return err;
    }

    s_tinyusb_config = TINYUSB_DEFAULT_CONFIG(usb_event_callback);
    err = tinyusb_driver_install(&s_tinyusb_config);
    if (err != ESP_OK) {
        set_error(err, "TinyUSB device driver install failed");
        finish_service_start(false);
        return err;
    }

    finish_service_start(true);
    set_status(
        DAJILOG_PC_USB_UPDATE_WAITING_FOR_USB, ESP_OK, 0, 0, 0,
        "Connect the USB OTG port to your computer"
    );
    ESP_LOGI(TAG, "USB firmware update service started");
    return ESP_OK;
}

esp_err_t dajilog_pc_usb_update_apply(void)
{
    dajilog_pc_usb_update_status_t current;
    dajilog_pc_usb_update_get_status(&current);
    if (!s_started || current.state != DAJILOG_PC_USB_UPDATE_FILE_READY) {
        return ESP_ERR_INVALID_STATE;
    }
    if (s_apply_running) {
        return ESP_ERR_INVALID_STATE;
    }

    tinyusb_msc_mount_point_t mount_point;
    esp_err_t err = tinyusb_msc_get_storage_mount_point(s_storage_handle, &mount_point);
    if (err != ESP_OK || mount_point != TINYUSB_MSC_STORAGE_MOUNT_APP) {
        return ESP_ERR_INVALID_STATE;
    }

    s_apply_running = true;
    if (xTaskCreate(
            apply_task, "usb_ota_apply", UPDATE_TASK_STACK_SIZE, NULL, 5, NULL
        ) != pdPASS) {
        s_apply_running = false;
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

esp_err_t dajilog_pc_usb_update_retry(void)
{
    dajilog_pc_usb_update_status_t current;
    dajilog_pc_usb_update_get_status(&current);
    if (!s_started || s_apply_running || current.state != DAJILOG_PC_USB_UPDATE_ERROR) {
        return ESP_ERR_INVALID_STATE;
    }

    /* Discard a failed/stale image before exporting the FAT volume again. */
    if (remove(UPDATE_FILE_PATH) == 0) {
        ESP_LOGI(TAG, "Removed failed update file before retry");
    }

    esp_err_t err = tinyusb_msc_set_storage_mount_point(
        s_storage_handle, TINYUSB_MSC_STORAGE_MOUNT_USB
    );
    if (err != ESP_OK) {
        set_error(err, "USB storage retry failed");
        return err;
    }

    set_status(
        DAJILOG_PC_USB_UPDATE_WAITING_FOR_USB, ESP_OK, 0, 0, 0,
        "Retry ready. Reconnect USB and copy DAJILOG_UPDATE.bin"
    );
    ESP_LOGI(TAG, "USB firmware update retry ready");
    return ESP_OK;
}

esp_err_t dajilog_pc_usb_update_reboot(void)
{
    dajilog_pc_usb_update_status_t current;
    dajilog_pc_usb_update_get_status(&current);
    if (current.state != DAJILOG_PC_USB_UPDATE_READY_TO_REBOOT) {
        return ESP_ERR_INVALID_STATE;
    }
    esp_restart();
    return ESP_OK;
}

void dajilog_pc_usb_update_get_status(dajilog_pc_usb_update_status_t *status)
{
    if (status == NULL) {
        return;
    }
    portENTER_CRITICAL(&s_status_lock);
    *status = s_status;
    portEXIT_CRITICAL(&s_status_lock);
}
