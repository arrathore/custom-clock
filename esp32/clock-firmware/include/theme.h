#ifndef THEME_H
#define THEME_H

#include <stdbool.h>
#include <stdint.h>

// data structure for images
typedef struct {
  uint16_t *pixels;
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

// get the image for a digit from the current theme
const theme_image_t *theme_getImage(int digit);

// set an image for a digit
bool theme_setImage(int digit, uint16_t* pixels, int width, int height);

// reset all theme assets to NULL
void theme_clear(void);

#endif
