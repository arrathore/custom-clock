#include "theme.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define THEME_DIGITS 10
#define THEME_PATH "/storage/theme"

static theme_image_t images[THEME_DIGITS];

static SemaphoreHandle_t theme_mutex;
void theme_lock(void) { xSemaphoreTake(theme_mutex, portMAX_DELAY); }
void theme_unlock(void) { xSemaphoreGive(theme_mutex); }

static FILE *write_file = NULL;
static int write_digit = -1;

static void getImagePath(int digit, char *path, size_t path_size) {
  snprintf(path, path_size, THEME_PATH "/%d.rgb565", digit);
}

void theme_init(void) {
  theme_mutex = xSemaphoreCreateMutex();

  mkdir(THEME_PATH, 0777);
  char image_path[64];
  for (int i = 0; i < THEME_DIGITS; i++) {
    getImagePath(i, image_path, sizeof(image_path));

    struct stat st;
    if (stat(image_path, &st) == 0)
      images[i].exists = true;
    else
      images[i].exists = false;

    images[i].width = 0;
    images[i].height = 0;
  }
}

bool theme_containsImage(int digit) {
  if (digit < 0 || digit >= THEME_DIGITS)
    return false;

  theme_lock();
  bool exists = images[digit].exists;
  theme_unlock();

  return exists;
}

// create or overwrite a file for writing
bool theme_beginWrite(int digit, int width, int height) {
  if (digit < 0 || digit >= THEME_DIGITS ||
      width <= 0 || height <= 0)
    return false;

  theme_lock();

  if (write_file != NULL) {
    theme_unlock();
    return false;
  }

  char image_path[64];
  getImagePath(digit, image_path, sizeof(image_path));

  write_file = fopen(image_path, "wb");
  if (write_file == NULL) {
    theme_unlock();
    return false;
  }

  write_digit = digit;

  // if the image previously existed, it doesn't anymore
  images[digit].exists = false;
  images[digit].width = width;
  images[digit].height = height;

  theme_unlock();
  return true;
}

// write data to the file
bool theme_write(const void *data, size_t size) {
  if (data == NULL || size == 0)
    return false;

  theme_lock();

  if (write_file == NULL) {
    theme_unlock();
    return false;
  }

  size_t written = fwrite(data, 1, size, write_file);
  bool success = written == size;

  theme_unlock();
  return success;
}

// complete upload, mark as available
bool theme_endWrite(void) {
  theme_lock();

  if (write_file == NULL) {
    theme_unlock();
    return false;
  }

  bool success = fclose(write_file) == 0;

  write_file = NULL;

  // mark image as readable
  if (success && write_digit >= 0) {
    images[write_digit].exists = true;
  }

  write_digit = -1;
  theme_unlock();

  return success;
}

bool theme_readImage(int digit, uint16_t *pixels, size_t pixel_count) {
  if (digit < 0 || digit >= THEME_DIGITS || pixels == NULL)
    return false;

  theme_lock();

  if (!images[digit].exists) {
    theme_unlock();
    return false;
  }

  size_t expected_size = (size_t)images[digit].width * images[digit].height * 2;

  if (pixel_count * sizeof(uint16_t) < expected_size) {
    theme_unlock();
    return false;
  }

  char image_path[64];
  getImagePath(digit, image_path, sizeof(image_path));
  FILE *file = fopen(image_path, "rb");

  if (file == NULL) {
    theme_unlock();
    return false;
  }

  uint8_t buffer[512];
  size_t pixel_index = 0;

  // read image into the buffer
  while (pixel_index * 2 < expected_size) {
    size_t remaining = expected_size - pixel_index * 2;
    size_t chunk_size = remaining < sizeof(buffer) ? remaining : sizeof(buffer);

    size_t bytes_read = fread(buffer, 1, chunk_size, file);

    if (bytes_read != chunk_size) {
      fclose(file);
      theme_unlock();
      return false;
    }

    // copy into pixels array
    for (size_t i = 0; i < bytes_read; i += 2)
      pixels[pixel_index++] = ((uint16_t)buffer[i] << 8) | buffer[i + 1];
  }

  fclose(file);
  theme_unlock();
  return true;
}

void theme_clear(void) {
  theme_lock();

  char image_path[64];
  for (int i = 0; i < THEME_DIGITS; i++) {
    getImagePath(i, image_path, sizeof(image_path));
    remove(image_path);

    images[i].exists = false;
    images[i].width = 0;
    images[i].height = 0;
  }

  theme_unlock();
}
