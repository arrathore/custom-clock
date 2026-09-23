#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stddef.h>

#include "esp_err.h"
#include "st7735.h"

// perform initialization tasks
void display_init(void);

// get the lcd object
lcd_st7735_t* display_getHandle(void);

// draw a bmp at x, y
esp_err_t display_drawBMP(lcd_st7735_t *lcd,
                          const uint8_t *bmp, size_t bmp_size,
                          int x, int y);

esp_err_t display_drawImage(lcd_st7735_t *lcd,
                            const uint16_t *pixels,
                            int width, int height,
			    int x, int y);

#endif
			    
