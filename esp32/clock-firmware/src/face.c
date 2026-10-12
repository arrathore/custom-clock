#include "face.h"

#include <stdlib.h>

#include "esp_log.h"

#include "display.h"
#include "theme.h"

static const char *TAG = "face";

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 160
#define DIGIT_COUNT 10

static bool s_use_24_hour;
static bool s_initialized;

static int s_digit_width;
static int s_digit_height;
static uint16_t *s_pixels;
static size_t s_pixel_count;

// returns {h, h, m, m}
static void timeToDigits(const struct tm *time_info, int digits[4])
{
    int hour = time_info->tm_hour;

    if (!s_use_24_hour) {
        hour %= 12;

        if (hour == 0) {
            hour = 12;
        }
    }

    const int minute = time_info->tm_min;

    digits[0] = hour / 10;
    digits[1] = hour % 10;
    digits[2] = minute / 10;
    digits[3] = minute % 10;
}

esp_err_t face_init(bool use_24_hour) {
  s_use_24_hour = use_24_hour;

  int width = 0;
  int height = 0;

  if (!theme_getImageInfo(0, &width, &height) ||
      width <= 0 || height <= 0) {
    ESP_LOGE(TAG, "Digit 0 is missing or has invalid dimensions");
    return ESP_ERR_NOT_FOUND;
  }

  s_digit_width = width;
  s_digit_height = height;

  // ensure every digit exists and uses the same dimensions. 
  for (int digit = 0; digit < DIGIT_COUNT; ++digit) {
    int digit_width = 0;
    int digit_height = 0;

    if (!theme_getImageInfo(digit, &digit_width, &digit_height)) {
      ESP_LOGE(TAG, "Digit %d is missing", digit);
      return ESP_ERR_NOT_FOUND;
    }

    if (digit_width != s_digit_width ||
	digit_height != s_digit_height) {
      ESP_LOGE(TAG,
	       "Digit %d has dimensions %dx%d; expected %dx%d",
	       digit, digit_width, digit_height,
	       s_digit_width, s_digit_height);
      return ESP_ERR_INVALID_SIZE;
    }
  }

  if (2 * s_digit_width > DISPLAY_WIDTH ||
      2 * s_digit_height > DISPLAY_HEIGHT) {
    ESP_LOGE(TAG,
	     "Digit images (%dx%d) are too large for %dx%d display",
	     s_digit_width, s_digit_height,
	     DISPLAY_WIDTH, DISPLAY_HEIGHT);
    return ESP_ERR_INVALID_SIZE;
  }

  s_pixel_count = (size_t)s_digit_width * (size_t)s_digit_height;
  s_pixels = malloc(s_pixel_count * sizeof(*s_pixels));

  if (s_pixels == NULL) {
    ESP_LOGE(TAG, "Failed to allocate digit image buffer");
    return ESP_ERR_NO_MEM;
  }

  s_initialized = true;

  ESP_LOGI(TAG, "Clock display initialized: %dx%d digits, %s mode",
	   s_digit_width, s_digit_height,
	   s_use_24_hour ? "24-hour" : "12-hour");

  return ESP_OK;
}

esp_err_t face_update(const struct tm *time_info) {
  if (!s_initialized || time_info == NULL) {
    return ESP_ERR_INVALID_STATE;
  }

  int digits[4];
  timeToDigits(time_info, digits);

  lcd_st7735_t *lcd = display_getHandle();

  if (lcd == NULL) {
    return ESP_ERR_INVALID_STATE;
  }

  const int start_x = (DISPLAY_WIDTH - 2 * s_digit_width) / 2;
  const int start_y = (DISPLAY_HEIGHT - 2 * s_digit_height) / 2;

  for (int i = 0; i < 4; ++i) {
    if (!theme_readImage(digits[i], s_pixels, s_pixel_count)) {
      ESP_LOGE(TAG, "Failed to read digit %d", digits[i]);
      return ESP_FAIL;
    }

    const int column = i % 2;
    const int row = i / 2;

    const int x = start_x + column * s_digit_width;
    const int y = start_y + row * s_digit_height;

    esp_err_t err = display_drawImage(
				      lcd, s_pixels,
				      s_digit_width, s_digit_height,
				      x, y);

    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to draw digit %d: %s",
	       digits[i], esp_err_to_name(err));
      return err;
    }
  }

  return ESP_OK;
}

