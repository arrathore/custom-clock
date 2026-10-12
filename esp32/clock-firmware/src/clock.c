#include "clock.h"

#include <stdlib.h>
#include <time.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "clock";

#define NTP_SERVER "pool.ntp.org"
#define SNTP_SYNC_TIMEOUT_MS 15000

static bool s_sntp_initialized = false;

bool clock_isSynced(void) {
  time_t now;
  time(&now);

  // treat timestamps before this date as an unsynchronized clock
  return now >= 1700000000; // nov 15, 2023
}

esp_err_t clock_init(void) {
  // TODO: get timezone automatically
  setenv("TZ", "CST6CDT,M3.2.0,M11.1.0", 1); // chicago
  tzset();

  esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(NTP_SERVER);

  esp_err_t err = esp_netif_sntp_init(&config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize SNTP: %s", esp_err_to_name(err));
    return err;
  }

  s_sntp_initialized = true;
  ESP_LOGI(TAG, "SNTP started using %s", NTP_SERVER);

  // obtain initial time
  err = esp_netif_sntp_sync_wait(
				 pdMS_TO_TICKS(SNTP_SYNC_TIMEOUT_MS)); // don't hold up rest of application

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "System time synchronized");
    } else {
        ESP_LOGW(TAG,
                 "Initial time synchronization timed out; "
                 "waiting for SNTP to synchronize in the background");
    }

    return ESP_OK;
  
}

esp_err_t clock_getTime(struct tm *out_time)
{
    if (out_time == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!clock_isSynced()) {
        return ESP_ERR_INVALID_STATE;
    }

    time_t now;
    time(&now);

    if (localtime_r(&now, out_time) == NULL) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

