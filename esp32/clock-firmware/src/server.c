#include "server.h"

#include <stdio.h>
#include <stdlib.h>

#include "esp_http_server.h"
#include "esp_log.h"

#include "display.h"

#define TAG "server"

#define IMAGE_WIDTH 30
#define IMAGE_HEIGHT 69
#define IMAGE_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT * 2)

static esp_err_t root_handler(httpd_req_t *req) {
  ESP_LOGI(TAG, "/ called");
  httpd_resp_set_type(req, "text/plain");
  httpd_resp_sendstr(req, "custom-clock is alive!\n");

  return ESP_OK;
}

static esp_err_t image_handler(httpd_req_t *req) {
  ESP_LOGI(TAG, "/image called");
  
  if (req->content_len != IMAGE_SIZE) {
    ESP_LOGE(TAG, "expected %d bytes, received %d", IMAGE_SIZE, req->content_len);

    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid image size");

    return ESP_FAIL;
  }

  uint8_t *data = malloc(IMAGE_SIZE);

  if (data == NULL) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
    return ESP_ERR_NO_MEM;
  }

  int received = 0;

  while (received < IMAGE_SIZE) {
    int ret =
        httpd_req_recv(req, (char *)data + received, IMAGE_SIZE - received);

    if (ret <= 0) {
      free(data);

      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                          "Failed to receive image");

      return ESP_FAIL;
    }

    received += ret;
  }

  ESP_LOGI(TAG, "Received image: %d bytes", received);

  // convert big-endian RGB565 to expected format
  uint16_t *pixels = malloc(IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(uint16_t));

  if (pixels == NULL) {
    free(data);
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
    return ESP_ERR_NO_MEM;
  }

  for (int i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++) {
    pixels[i] =
      ((uint16_t)data[i * 2] << 8) | data[i * 2 + 1];
  }

  free(data);
  lcd_st7735_t *lcd = display_getHandle();

  if (lcd == NULL) {
    free(pixels);

    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
			"Display not initialized");

    return ESP_FAIL;
  }

  // replace with more efficient function later
  for (int y = 0; y < IMAGE_HEIGHT; y++)
    for (int x = 0; x < IMAGE_WIDTH; x++) {
      lcd_st7735_draw_pixel(lcd, 49 + x, 45 + y, pixels[y * IMAGE_WIDTH + x]);
    }

  free(pixels);
  httpd_resp_set_type(req, "text/plain");
  httpd_resp_sendstr(req, "OK");

  return ESP_OK;
}

void server_init(void) {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  httpd_handle_t server = NULL;
  esp_err_t err = httpd_start(&server, &config);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start HTTP server: %s", esp_err_to_name(err));
    return;
  }

  httpd_uri_t root_uri = {
      .uri = "/",
      .method = HTTP_GET,
      .handler = root_handler,
      .user_ctx = NULL,
  };
  httpd_register_uri_handler(server, &root_uri);

  httpd_uri_t image_uri = {
    .uri = "/image",
    .method = HTTP_POST,
    .handler = image_handler,
    .user_ctx = NULL
  };
  httpd_register_uri_handler(server, &image_uri);

  ESP_LOGI(TAG, "HTTP server started");
}

