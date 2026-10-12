#ifndef CLOCK_H
#define CLOCK_H

#include <stdbool.h>
#include <time.h>

#include "esp_err.h"

esp_err_t clock_init(void);

bool clock_isSynced(void);

esp_err_t clock_getTime(struct tm *out_time);

#endif

