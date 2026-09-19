#include "display.h"
#include "pins.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#define LCD_HOST SPI2_HOST

static lcd_st7735_t* lcd = NULL;

void display_init(void) {
  printf("Initializing display...\n");

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

  if (err != ESP_OK) {
    return;
  }

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

  err = lcd_st7735_new(&lcd_config, &lcd);

  printf("LCD result: %s\n", esp_err_to_name(err));

  if (err != ESP_OK) {
    lcd = NULL;
    return;
  }

  printf("LCD initialized!\n");

  err = lcd_st7735_display_on(lcd, true);

  printf("Display on: %s\n", esp_err_to_name(err));

  err = lcd_st7735_fill_screen(
			       lcd,
			       LCD_ST7735_RGB565(0, 0, 0)
			       );

  printf("Fill result: %s\n", esp_err_to_name(err));
}

lcd_st7735_t* display_getHandle(void) {
  return lcd;
}

esp_err_t display_drawBMP(lcd_st7735_t *lcd,
                                 const uint8_t *bmp, size_t bmp_size,
                                 int x, int y) {
  if (bmp_size < 54)
    return ESP_ERR_INVALID_SIZE;

  // check BMP signature
  if (bmp[0] != 'B' || bmp[1] != 'M')
    return ESP_ERR_INVALID_ARG;

  // BMP file header.
  uint32_t data_offset =
    ((uint32_t)bmp[10]) |
    ((uint32_t)bmp[11] << 8) |
    ((uint32_t)bmp[12] << 16) |
    ((uint32_t)bmp[13] << 24);

  // DIB header.
  uint32_t width =
    ((uint32_t)bmp[18]) |
    ((uint32_t)bmp[19] << 8) |
    ((uint32_t)bmp[20] << 16) |
    ((uint32_t)bmp[21] << 24);

  int32_t height =
    ((int32_t)bmp[22]) |
    ((int32_t)bmp[23] << 8) |
    ((int32_t)bmp[24] << 16) |
    ((int32_t)bmp[25] << 24);

  uint16_t bits_per_pixel =
    ((uint16_t)bmp[28]) |
    ((uint16_t)bmp[29] << 8);

  if (bits_per_pixel != 24) {
    return ESP_ERR_NOT_SUPPORTED;
  }

  bool bottom_up = height > 0;

  if (height < 0) {
    height = -height;
  }

  // BMP rows are padded to a 4-byte boundary.
  size_t row_size = ((width * 3 + 3) / 4) * 4;

  if (data_offset + row_size * height > bmp_size) {
    return ESP_ERR_INVALID_SIZE;
  }

  for (int row = 0; row < height; row++) {
    int bmp_row = bottom_up ? height - 1 - row : row;

    const uint8_t *src =
      bmp + data_offset + bmp_row * row_size;

    for (int col = 0; col < width; col++) {
      // BMP stores pixels as BGR.
      uint8_t b = src[col * 3 + 0];
      uint8_t g = src[col * 3 + 1];
      uint8_t r = src[col * 3 + 2];

      uint16_t color = LCD_ST7735_RGB565(r, g, b);

      lcd_st7735_draw_pixel(
			    lcd,
			    x + col,
			    y + row,
			    color
			    );
    }
  }

  return ESP_OK;
}
