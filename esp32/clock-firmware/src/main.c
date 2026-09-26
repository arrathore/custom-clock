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

static const char *TAG = "main";

// TODO: TEMP
#define THEME_WIDTH 70
#define THEME_HEIGHT 160

#define THEME_PIXEL_COUNT (THEME_WIDTH * THEME_HEIGHT)

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

  // allocate enough memory for one theme digit
  uint16_t *pixels = malloc(THEME_PIXEL_COUNT * sizeof(uint16_t));
  if (pixels == NULL) {
    ESP_LOGE(TAG, "Failed to allocate theme image buffer");
    return;
  }
  
  wifi_init();
  server_init();

  while (1) {
    for (int i = 0; i < 10; i++) {
      vTaskDelay(pdMS_TO_TICKS(500));
      if (!theme_containsImage(i)) {
	continue;
      }

      if (!theme_readImage(i, pixels, THEME_PIXEL_COUNT)) {
	continue;
      }

      ESP_LOGI(TAG, "Displaying theme digit %d", i);

      display_drawImage(display_getHandle(), pixels, THEME_WIDTH, THEME_HEIGHT, 0, 0);
    }
  }
}
