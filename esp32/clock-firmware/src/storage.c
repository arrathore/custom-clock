#include "storage.h"

#include "esp_littlefs.h"
#include "esp_log.h"

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#define STORAGE_PARTITION "storage"
#define STORAGE_BASE_PATH "/storage"
#define WEB_PARTITION "web"
#define WEB_BASE_PATH "/www"

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

  esp_vfs_littlefs_conf_t web_conf = {
    .base_path = WEB_BASE_PATH,
    .partition_label = WEB_PARTITION,
    .format_if_mount_failed = false,
    .dont_mount = false,
  };

  err = esp_vfs_littlefs_register(&web_conf);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to mount Web LittleFS: %s", esp_err_to_name(err));
    return err;
  }

  size_t total = 0;
  size_t used = 0;

  // Also verify storage
  err = esp_littlefs_info(STORAGE_PARTITION, &total, &used);
  if (err == ESP_OK) {
      ESP_LOGI(TAG, "Storage mounted. Total: %zu, Used: %zu", total, used);
  }

  err = esp_littlefs_info(WEB_PARTITION, &total, &used);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get filesystem info for WEB: %s", esp_err_to_name(err));
    return err;
  }

  ESP_LOGI(TAG, "Web LittleFS mounted");
  ESP_LOGI(TAG, "Total: %zu bytes", total);
  ESP_LOGI(TAG, "Used: %zu bytes", used);

  // TODO: TEMP TESTING CODE
  struct stat st;

  if (stat("/www/index.html", &st) == 0) {
    ESP_LOGI(TAG, "Found /www/index.html (%ld bytes)", (long)st.st_size);
  } else {
    ESP_LOGE(TAG, "Cannot access /www/index.html: errno=%d (%s)",
             errno, strerror(errno));
  }

  return ESP_OK;
}
