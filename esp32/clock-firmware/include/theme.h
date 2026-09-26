#ifndef THEME_H
#define THEME_H

#include <stdbool.h>
#include <stdint.h>

// data structure for images
typedef struct {
  bool exists;
  int width;
  int height;
} theme_image_t;

// creates a mutex for assets and initializes all digits to NULL
void theme_init(void);

// take and give the theme mutex
void theme_lock(void);
void theme_unlock(void);

// check if the currently loaded theme has an asset for a digit
bool theme_containsImage(int digit);

// set an image for a digit
bool theme_beginWrite(int digit, int width, int height);
bool theme_write(const void *data, size_t size);
bool theme_endWrite(void);

// get an image out of the theme
bool theme_readImage(int digit, uint16_t *pixels, size_t pixel_count);

// reset all theme assets
void theme_clear(void);

#endif
