#include "storage.h"

#include "esp_littlefs.h"
#include "esp_log.h"

#define STORAGE_PARTITION "storage"
#define STORAGE_BASE_PATH "/storage"

static const char *TAG = "storage";

esp_err_t storage_init(void) {
  esp_vfs_littlefs_conf_t conf = {
    .base_path = STORAGE_BASE_PATH,
    .partition_label = STORAGE_PARTITION,
    .format_if_mount_failed = false,
    .dont_mount = false,
  };

  esp_err_t err = esp_vfs_littlefs_register(&conf);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to mount LittleFS: %s", esp_err_to_name(err));
    return err;
  }

  size_t total = 0;
  size_t used = 0;

  err = esp_littlefs_info(STORAGE_PARTITION, &total, &used);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get filesystem info: %s", esp_err_to_name(err));
    return err;
  }

  ESP_LOGI(TAG, "LittleFS mounted");
  ESP_LOGI(TAG, "Total: %zu bytes", total);
  ESP_LOGI(TAG, "Used: %zu bytes", used);

  return ESP_OK;
}
