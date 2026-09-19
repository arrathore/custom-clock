#include "wifi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "wifi_config.h"

#define TAG "wifi"

#define WIFI_CONNECTED_BIT BIT0

static EventGroupHandle_t wifi_event_group;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
			       int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT &&
      event_id == WIFI_EVENT_STA_START) {

    esp_wifi_connect();

  } else if (event_base == WIFI_EVENT &&
	     event_id == WIFI_EVENT_STA_DISCONNECTED) {

    ESP_LOGI(TAG, "Disconnected, reconnecting...");
    esp_wifi_connect();

    xEventGroupClearBits(
			 wifi_event_group,
			 WIFI_CONNECTED_BIT
			 );

  } else if (event_base == IP_EVENT &&
	     event_id == IP_EVENT_STA_GOT_IP) {

    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

    ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

    xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

void wifi_init(void) {
  ESP_ERROR_CHECK(nvs_flash_init());

  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_create_default_wifi_sta();

  wifi_event_group = xEventGroupCreate();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(
		  esp_event_handler_register(
					     WIFI_EVENT,
					     ESP_EVENT_ANY_ID,
					     &wifi_event_handler,
					     NULL
					     )
		  );

  ESP_ERROR_CHECK(
		  esp_event_handler_register(
					     IP_EVENT,
					     IP_EVENT_STA_GOT_IP,
					     &wifi_event_handler,
					     NULL
					     )
		  );

  wifi_config_t wifi_config = {
    .sta = {
      .ssid = WIFI_SSID,
      .password = WIFI_PASSWORD,
    },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

  ESP_ERROR_CHECK(
		  esp_wifi_set_config(
				      WIFI_IF_STA,
				      &wifi_config
				      )
		  );

  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGI(TAG, "Wi-Fi initialization complete");

  xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE,
                      portMAX_DELAY);
  ESP_LOGI(TAG, "Wi-Fi connected");
}

