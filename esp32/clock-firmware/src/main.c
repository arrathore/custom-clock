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
  // storage test
  FILE *file = fopen("/storage/test.txt", "w");
  if (file == NULL) {
    ESP_LOGE("TEST", "Failed to open test.txt");
  } else {
    fprintf(file, "Hello from LittleFS!\n");
    fclose(file);

    ESP_LOGI("TEST", "Successfully wrote test.txt");
  }
  
  theme_init();
  wifi_init();
  server_init();

  while (1) {
    for (int i = 0; i < 10; i++) {
      theme_lock();
      
      const theme_image_t *image = theme_getImage(i);
      if (image != NULL) {
        display_drawImage(display_getHandle(), 
                          image->pixels, image->width, image->height,
                          0, 0);
      }

      theme_unlock();

      vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
}
