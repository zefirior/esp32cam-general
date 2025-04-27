#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_http_server.h"

// Initialize and start the HTTP server
void http_server_init(void);

// Stop the HTTP server
void http_server_stop(void);

#endif // HTTP_SERVER_H 