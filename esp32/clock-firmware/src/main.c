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

#include "0_bmp.h"

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
  // display_drawBMP(lcd, __0_bmp, __0_bmp_len, 49, 45);

  wifi_init();
  server_init();

  while (1) {
    /*
    printf("ALIVE\n");
    fflush(stdout);
    */
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
