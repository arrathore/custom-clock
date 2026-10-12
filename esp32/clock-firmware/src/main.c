#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "driver/spi_master.h"
#include "st7735.h"

#include "pins.h"
#include "display.h"
#include "wifi.h"
#include "server.h"
#include "theme.h"
#include "storage.h"
#include "clock.h"
#include "face.h"

#define THEME_DIGITS 10
#define  CLOCK_USE_24_HOUR true

static const char *TAG = "main";

// display test
void disp_test() {
  lcd_st7735_t *lcd = display_getHandle();
  lcd_st7735_fill_screen(lcd, 0xF800); // red
  vTaskDelay(pdMS_TO_TICKS(1000));

  lcd_st7735_fill_screen(lcd, 0x07E0); // green
  vTaskDelay(pdMS_TO_TICKS(1000));

  lcd_st7735_fill_screen(lcd, 0x001F); // blue
  vTaskDelay(pdMS_TO_TICKS(1000));

  lcd_st7735_fill_screen(lcd, 0x0000); // black  
}

// storage test
void store_test() {
  FILE *file = fopen("/storage/test.txt", "w");
  if (file == NULL) {
    ESP_LOGE("TEST", "Failed to open test.txt");
  } else {
    fprintf(file, "Hello from LittleFS!\n");
    fclose(file);

    ESP_LOGI("TEST", "Successfully wrote test.txt");
  }  
}

void app_main(void) {
  vTaskDelay(pdMS_TO_TICKS(5000)); // delay for serial output
  printf("START\n");
  fflush(stdout);

  display_init();
  esp_err_t err = storage_init();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Storage initialization failed: %s", esp_err_to_name(err));
    return;
  }
  theme_init();

  /*
  // find the dimensions of the first available digit
  int theme_width = 0;
  int theme_height = 0;

  for (int i = 0; i < THEME_DIGITS; i++)
    if (theme_getImageInfo(i, &theme_width, &theme_height))
      break;

  uint16_t *pixels = NULL;

  if (theme_width > 0 && theme_height > 0) {
    size_t pixel_count = (size_t)theme_width * theme_height;

    pixels = malloc(pixel_count * sizeof(uint16_t));

    if (pixels == NULL) {
      ESP_LOGE(TAG, "Failed to allocate theme buffer (%zu pixels)", pixel_count);
      return;
    }

    ESP_LOGI(TAG, "Theme: %dx%d (%zu pixels, %zu bytes)",
	     theme_width, theme_height,
	     pixel_count, pixel_count * sizeof(uint16_t));
  } else {
    ESP_LOGI(TAG, "No theme images found");
  }
  */

  err = face_init(CLOCK_USE_24_HOUR);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Clock face initialization failed: %s", esp_err_to_name(err));
    return;
  }

  wifi_init();
  server_init();

  err = clock_init();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Clock initialization failed: %s", esp_err_to_name(err));
    return;
  }

  int last_hour = -1;
  int last_minute = -1;

  while (1) {
    struct tm now;

    if (clock_getTime(&now) == ESP_OK) {
      if (now.tm_hour != last_hour ||
	  now.tm_min != last_minute) {

	err = face_update(&now);

	if (err == ESP_OK) {
	  last_hour = now.tm_hour;
	  last_minute = now.tm_min;

	  ESP_LOGI(TAG, "Displayed time: %02d:%02d",
		   now.tm_hour, now.tm_min);
	} else {
	  ESP_LOGE(TAG, "Display update failed: %s",
		   esp_err_to_name(err));
	}
      }
    } else {
      // Wait for SNTP to establish a valid system time.
      ESP_LOGD(TAG, "Waiting for synchronized time");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
    
    /*
    for (int i = 0; i < THEME_DIGITS; i++) {
      vTaskDelay(pdMS_TO_TICKS(500));

      // no theme loaded
      if (pixels == NULL) {
	ESP_LOGI(TAG, "No theme loaded");
	break;
      }

      // digit does not exist
      if (!theme_containsImage(i)) {
	ESP_LOGI(TAG, "Digit %d does not exist", i);
	continue;
      }

      // failed to read digit
      if (!theme_readImage(i, pixels, (size_t)theme_width * theme_height)) {
	ESP_LOGI(TAG, "Failed to read digit %d", i);
	continue;
      }

      ESP_LOGI(TAG, "Displaying theme digit %d", i);

      display_drawImage(display_getHandle(), pixels, theme_width, theme_height, 0, 0);
    }
    */
  }
}
