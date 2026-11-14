#include <esp_http_server.h>
#include <esp_log.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define TAG "page_server"
#define WEB_PATH "/www"

static esp_err_t file_handler(httpd_req_t *req) {
    char filepath[256];
    size_t uri_len = strlen(req->uri);
    const char *target_file = (strcmp(req->uri, "/") == 0) ? "/index.html" : req->uri;
    
    if (snprintf(filepath, sizeof(filepath), "%s%s", WEB_PATH, target_file) >= sizeof(filepath)) {
        ESP_LOGE(TAG, "URI too long: %s", req->uri);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "URI too long");
        return ESP_FAIL;
    }

    struct stat st;
    if (stat(filepath, &st) != 0) {
        ESP_LOGE(TAG, "File not found: %s", filepath);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
        return ESP_FAIL;
    }

    FILE *f = fopen(filepath, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file: %s", filepath);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read file");
        return ESP_FAIL;
    }

    // Guess content type based on extension (simplified)
    if (strstr(filepath, ".html")) httpd_resp_set_type(req, "text/html");
    else if (strstr(filepath, ".css")) httpd_resp_set_type(req, "text/css");
    else if (strstr(filepath, ".js")) httpd_resp_set_type(req, "application/javascript");
    else if (strstr(filepath, ".ico")) httpd_resp_set_type(req, "image/x-icon");

    char buffer[1024];
    size_t read;
    while ((read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        httpd_resp_send_chunk(req, buffer, read);
    }
    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

void register_page_server(httpd_handle_t server) {
    httpd_uri_t file_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = file_handler,
        .user_ctx = NULL,
    };
    httpd_register_uri_handler(server, &file_uri);
    ESP_LOGI(TAG, "File server registered at /* serving from %s", WEB_PATH);
}
