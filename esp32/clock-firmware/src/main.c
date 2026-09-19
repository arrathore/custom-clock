#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
  printf("hello world\n");
  
  while (1) {
    printf("hello again\n");
    fflush(stdout);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
