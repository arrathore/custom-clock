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

#define THEME_DIGITS 10

static const char *TAG = "main";
void app_main(void) {
  vTaskDelay(pdMS_TO_TICKS(5000)); // delay for serial output
  printf("START\n");
  fflush(stdout);

  display_init();
  // display test
  /*
    lcd_st7735_t *lcd = display_getHandle();
    lcd_st7735_fill_screen(lcd, 0xF800); // red
    vTaskDelay(pdMS_TO_TICKS(1000));

    lcd_st7735_fill_screen(lcd, 0x07E0); // green
    vTaskDelay(pdMS_TO_TICKS(1000));

    lcd_st7735_fill_screen(lcd, 0x001F); // blue
    vTaskDelay(pdMS_TO_TICKS(1000));

    lcd_st7735_fill_screen(lcd, 0x0000); // black
  */

  ESP_ERROR_CHECK(storage_init());
  /*
  // storage test
  FILE *file = fopen("/storage/test.txt", "w");
  if (file == NULL) {
    ESP_LOGE("TEST", "Failed to open test.txt");
  } else {
    fprintf(file, "Hello from LittleFS!\n");
    fclose(file);

    ESP_LOGI("TEST", "Successfully wrote test.txt");
  }
  */
  
  theme_init();
  
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
  
  wifi_init();
  server_init();

  while (1) {
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
  }
}
