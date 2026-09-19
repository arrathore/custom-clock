#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "driver/spi_master.h"
#include "st7735.h"


#include "pins.h"

#define LCD_HOST SPI2_HOST

void app_main(void) {
  vTaskDelay(pdMS_TO_TICKS(5000)); // delay for serial output
  printf("START\n");
  fflush(stdout);

  spi_bus_config_t bus_config = {
    .mosi_io_num = PIN_SPI_MOSI,
    .miso_io_num = -1,
    .sclk_io_num = PIN_SPI_SCK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 128 * 160 * 2 + 8,
  };

  esp_err_t err = spi_bus_initialize(
				     LCD_HOST,
				     &bus_config,
				     SPI_DMA_CH_AUTO
				     );

  printf("SPI result: %s\n", esp_err_to_name(err));
  fflush(stdout);

  if (err != ESP_OK) return;

  lcd_st7735_config_t lcd_config = {
    .spi_host = LCD_HOST,

    .pin_cs = PIN_TFT_CS,
    .pin_dc = PIN_TFT_DC,
    .pin_rst = PIN_TFT_RST,
    .pin_bl = GPIO_NUM_NC,

    .width = 128,
    .height = 160,

    .x_offset = 0,
    .y_offset = 0,

    .spi_clock_hz = 10 * 1000 * 1000,

    .color_order = LCD_ST7735_COLOR_ORDER_RGB,
    .invert_color = false,
    .reset_level = 0,
  };

  lcd_st7735_t *lcd = NULL;
  err = lcd_st7735_new(&lcd_config, &lcd);

  printf("LCD result: %s\n", esp_err_to_name(err));
  fflush(stdout);

  if (err != ESP_OK) return;

  printf("LCD initialized!\n");
  fflush(stdout);

  // fill screen red
  err = lcd_st7735_fill_screen(lcd, LCD_ST7735_RGB565(255, 0, 0));

  printf("Fill result: %s\n", esp_err_to_name(err));

  while (1) {
    printf("ALIVE\n");
    fflush(stdout);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
