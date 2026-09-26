#include "server.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "esp_http_server.h"
#include "esp_log.h"

#include "display.h"
#include "theme.h"

#define TAG "server"

#define MAX_IMAGE_WIDTH 128
#define MAX_IMAGE_HEIGHT 160

#define THEME_WRITE_BUFFER_SIZE 512

// /
static esp_err_t root_handler(httpd_req_t *req) {
  ESP_LOGI(TAG, "/ called");
  httpd_resp_set_type(req, "text/plain");
  httpd_resp_sendstr(req, "custom-clock is alive!\n");

  return ESP_OK;
}

static esp_err_t receive_rgb565_image(httpd_req_t *req, uint16_t **pixels_out,
                                      int *width_out, int *height_out) {
  // validate args
  char width_str[16];
  char height_str[16];
  if (httpd_req_get_hdr_value_str(req, "X-Image-Width",
                                  width_str, sizeof(width_str)) != ESP_OK)
    return ESP_ERR_INVALID_ARG;

  if (httpd_req_get_hdr_value_str(req, "X-Image-Height",
                                  height_str, sizeof(height_str)) != ESP_OK)
    return ESP_ERR_INVALID_ARG;

  // validate dimensions
  int width = atoi(width_str);
  int height = atoi(height_str);
  if (width <= 0 || height <= 0)
    return ESP_ERR_INVALID_ARG;

  // validate size
  size_t image_size = (size_t)width * height * 2;
  if (req->content_len != image_size) {
    ESP_LOGE(TAG, "Expected %u bytes, received %d",
             (unsigned)image_size, req->content_len);

    return ESP_ERR_INVALID_SIZE;
  }


  uint8_t *data = malloc(image_size);
  if (data == NULL)
    return ESP_ERR_NO_MEM;

  // extract data
  int received = 0;
  while (received < image_size) {
    int ret =
        httpd_req_recv(req, (char *)data + received, image_size - received);

    if (ret <= 0) {
      ESP_LOGE(TAG, "httpd_req_recv failed: ret=%d, received=%d/%zu",
               ret, received, image_size);

      free(data);
      return ESP_FAIL;
    }

    received += ret;
  }

  uint16_t *pixels = malloc((size_t)width * height * sizeof(uint16_t));
  if (pixels == NULL) {
    free(data);
    ESP_LOGE(TAG, "Out of memory!");
    return ESP_ERR_NO_MEM;
  }

  for (size_t i = 0; i < (size_t)width * height; i++) {
    pixels[i] = ((uint16_t)data[i * 2] << 8) | data[i * 2 + 1];
  }

  free(data);

  *pixels_out = pixels;
  *width_out = width;
  *height_out = height;

  return ESP_OK;
}

// /image OPTIONS
static esp_err_t image_options_handler(httpd_req_t *req) {
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Methods",
                     "POST, OPTIONS");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Headers",
                     "Content-Type, X-Image-Width, X-Image-Height");

  httpd_resp_send(req, NULL, 0);

  return ESP_OK;
}

// /image
static esp_err_t image_handler(httpd_req_t *req) {
  ESP_LOGI(TAG, "/image called");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  char width_str[8];
  char height_str[8];

  if (httpd_req_get_hdr_value_str(req, "X-Image-Width", width_str, sizeof(width_str)) != ESP_OK ||
      httpd_req_get_hdr_value_str(req, "X-Image-Height", height_str, sizeof(height_str)) != ESP_OK) {

    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing image dimensions");

    return ESP_FAIL;
  }

  int width = atoi(width_str);
  int height = atoi(height_str);

  ESP_LOGI(TAG, "Image: %d x %d", width, height);

  // validate dimensions
  if (width <= 0 || height <= 0 ||
      width > MAX_IMAGE_WIDTH || height > MAX_IMAGE_HEIGHT) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid image dimensions");

    return ESP_FAIL;
  }

  size_t image_size = (size_t)width * height * 2;
  if (req->content_len != image_size) {
    ESP_LOGE(TAG, "Expected %zu bytes, received %d", image_size, req->content_len);
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid image size");

    return ESP_FAIL;
  }

  uint8_t* data = malloc(image_size);
  if (data == NULL) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
    return ESP_ERR_NO_MEM;
  }

  int received = 0;
  while (received < image_size) {
    int ret =
        httpd_req_recv(req, (char *)data + received, image_size - received);

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
  uint16_t *pixels = malloc(width * height * sizeof(uint16_t));
  if (pixels == NULL) {
    free(data);
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
    return ESP_ERR_NO_MEM;
  }

  for (int i = 0; i < width * height; i++) {
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

  // TODO: replace with more efficient function
  for (int y = 0; y < height; y++)
    for (int x = 0; x < width; x++) {
      lcd_st7735_draw_pixel(lcd, x, y, pixels[y * width + x]);
    }

  free(pixels);

  //httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_type(req, "text/plain");
  httpd_resp_sendstr(req, "OK");

  return ESP_OK;
}

// /theme/*
static esp_err_t theme_image_handler(httpd_req_t *req) {
  ESP_LOGI(TAG, "/theme called");
  
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  // extract digit
  const char *uri = req->uri;
  const char *prefix = "/theme/";
  if (strncmp(uri, prefix, strlen(prefix)) != 0) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid theme URI");
    return ESP_FAIL;
  }
  const char *digit_str = uri + strlen(prefix);

  // validate digit
  if (digit_str[0] < '0' || digit_str[0] > '9' || digit_str[1] != '\0') {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Theme digit must be 0-9");
    return ESP_FAIL;
  }
  int digit = digit_str[0] - '0';

  // get dimensions
  char width_str[16];
  char height_str[16];
  
  if (httpd_req_get_hdr_value_str(req, "X-Image-Width", // width exists
				  width_str, sizeof(width_str)) != ESP_OK ||  
      httpd_req_get_hdr_value_str(req, "X-Image-Height", // height exists
				  height_str, sizeof(height_str)) != ESP_OK) { 
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing image dimensions");
    return ESP_FAIL;
  }

  int width = atoi(width_str);
  int height = atoi(height_str);

  ESP_LOGI(TAG, "Theme digit %d: %d x %d", digit, width, height);

  // validate dimensions
  if (width <= 0 || height <= 0 ||
      width > MAX_IMAGE_WIDTH || height > MAX_IMAGE_HEIGHT) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid image dimensions");
    return ESP_FAIL;
  }

  // validate request size
  size_t image_size = (size_t)width * height * 2;

  if (req->content_len != image_size) {
    ESP_LOGE(TAG, "Expected %zu bytes, received %d", image_size, req->content_len);

    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid image size");
    return ESP_FAIL;
  }

  // start writing the image to LittleFS
  if (!theme_beginWrite(digit, width, height)) {
    ESP_LOGE(TAG, "Failed to begin theme write");

    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to open theme file");
    return ESP_FAIL;
  }

  uint8_t buffer[THEME_WRITE_BUFFER_SIZE];
  size_t received = 0;

  while (received < image_size) {
    size_t remaining = image_size - received;
    size_t to_receive = remaining < sizeof(buffer) ? remaining : sizeof(buffer);

    int ret = httpd_req_recv(req, (char *)buffer, to_receive);

    if (ret <= 0) {
      ESP_LOGE(TAG, "httpd_req_recv failed: ret=%d, received=%zu/%zu",
	       ret, received, image_size);

      // close the file without marking the image as valid
      theme_endWrite();
      
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive image");
      return ESP_FAIL;
    }

    if (!theme_write(buffer, ret)) {
      ESP_LOGE(TAG, "Failed to write theme data");

      theme_endWrite();

      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to write theme");
      return ESP_FAIL;
    }

    received += ret;
  }

  // finish the file
  if (!theme_endWrite()) {
    ESP_LOGE(TAG, "Faild to finish theme write");
    
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to finish theme");
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "Stored theme digit %d (%dx%d, %zu bytes)",
	   digit, width, height, received);

  httpd_resp_set_status(req, "200 OK");
  httpd_resp_set_type(req, "text/plain");
  httpd_resp_sendstr(req, "OK");

  return ESP_OK;
}

// /theme OPTIONS
static esp_err_t theme_options_handler(httpd_req_t *req) {
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Methods",
                     "POST, OPTIONS");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Headers",
                     "Content-Type, X-Image-Width, X-Image-Height");

  httpd_resp_set_status(req, "204 No Content");
  return httpd_resp_send(req, NULL, 0);
}

void server_init(void) {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.uri_match_fn = httpd_uri_match_wildcard;

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
  httpd_uri_t image_options_uri = {
      .uri = "/image",
      .method = HTTP_OPTIONS,
      .handler = image_options_handler,
      .user_ctx = NULL,
  };
  httpd_register_uri_handler(server, &image_uri);
  httpd_register_uri_handler(server, &image_options_uri);

  httpd_uri_t theme_image_uri = {
      .uri = "/theme/*",
      .method = HTTP_POST,
      .handler = theme_image_handler,
      .user_ctx = NULL,
  };
  httpd_uri_t theme_options_uri = {
      .uri = "/theme/*",
      .method = HTTP_OPTIONS,
      .handler = theme_options_handler,
      .user_ctx = NULL,
  };
  httpd_register_uri_handler(server, &theme_image_uri);
  httpd_register_uri_handler(server, &theme_options_uri);
  
  ESP_LOGI(TAG, "HTTP server started");
}

