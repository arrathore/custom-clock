#ifndef SERVER_H
#define SERVER_H

#include "esp_http_server.h"

// register endpoints
void server_init(void);
void register_page_server(httpd_handle_t server);

#endif
