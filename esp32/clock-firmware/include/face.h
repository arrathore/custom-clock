#ifndef FACE_H
#define FACE_H

#include <time.h>
#include <stdbool.h>

#include "esp_err.h"

esp_err_t face_init(bool use_24_hour);

esp_err_t face_update(const struct tm *time_info);

#endif

