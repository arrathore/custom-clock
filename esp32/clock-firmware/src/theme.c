#include "theme.h"

#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define THEME_DIGITS 10

static theme_image_t images[THEME_DIGITS];

static SemaphoreHandle_t theme_mutex;
void theme_lock(void) { xSemaphoreTake(theme_mutex, portMAX_DELAY); }
void theme_unlock(void) { xSemaphoreGive(theme_mutex); }

void theme_init(void) {
  theme_mutex = xSemaphoreCreateMutex();
  for (int i = 0; i < THEME_DIGITS; i++) {
    images[i].pixels = NULL;
    images[i].width = 0;
    images[i].height = 0;
  }
}

bool theme_containsImage(int digit) {
  if (digit < 0 || digit >= THEME_DIGITS)
    return false;

  xSemaphoreTake(theme_mutex, portMAX_DELAY);
  bool exists = images[digit].pixels != NULL;
  xSemaphoreGive(theme_mutex);

  return exists;
}

const theme_image_t *theme_getImage(int digit) {
  if (digit < 0 || digit >= THEME_DIGITS)
    return NULL;

  return &images[digit];
}

bool theme_setImage(int digit, uint16_t *pixels, int width, int height) {
  if (digit < 0 || digit >= THEME_DIGITS ||
      pixels == NULL ||
      width <= 0 ||height <= 0) return false;

  // free the existing data
  xSemaphoreTake(theme_mutex, portMAX_DELAY);
  free(images[digit].pixels);

  // set new data
  images[digit].pixels = pixels;
  images[digit].width = width;
  images[digit].height = height;
  xSemaphoreGive(theme_mutex);
  
  return true;
}

void theme_clear(void) {
  xSemaphoreTake(theme_mutex, portMAX_DELAY);

  for (int i = 0; i < THEME_DIGITS; i++) {
    free(images[i].pixels);

    images[i].pixels = NULL;
    images[i].width = 0;
    images[i].height = 0;
  }

  xSemaphoreGive(theme_mutex);
}
