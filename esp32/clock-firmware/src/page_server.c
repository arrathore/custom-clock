#include <esp_http_server.h>
#include <esp_log.h>

#define TAG "page_server"

static const char *hello_html = 
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    "    <meta charset=\"UTF-8\">\n"
    "    <title>Clock Hello World</title>\n"
    "    <style>\n"
    "        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #f0f2f5; color: #333; }\n"
    "        h1 { color: #0066cc; }\n"
    "        p { font-size: 1.2em; }\n"
    "    </style>\n"
    "</head>\n"
    "<body>\n"
    "    <h1>Hello World from ESP32 Clock!</h1>\n"
    "    <p>Welcome to your custom clock firmware web server.</p>\n"
    "</body>\n"
    "</html>\n";

static esp_err_t hello_page_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "GET / requested");
    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr(req, hello_html);
    return ESP_OK;
}

void register_page_server(httpd_handle_t server) {
    httpd_uri_t hello_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = hello_page_handler,
        .user_ctx = NULL,
    };
    httpd_register_uri_handler(server, &hello_uri);
    ESP_LOGI(TAG, "Hello world page server endpoints registered at /");
}
