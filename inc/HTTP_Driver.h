#ifndef HTTP_DRIVER_H
#define HTTP_DRIVER_H

#include "esp_err.h"
#include "esp_http_server.h"

httpd_handle_t HTTP_Start(void);
esp_err_t HTTP_Stop(httpd_handle_t server);
esp_err_t HTTP_RegisterRoot(httpd_handle_t server, esp_err_t (*handler)(httpd_req_t *req));
esp_err_t HTTP_RegisterUri(httpd_handle_t server, const char *uri, httpd_method_t method, esp_err_t (*handler)(httpd_req_t *req));

#endif // HTTP_DRIVER_H
