#include "HTTP_Driver.h"
#include <string.h>

httpd_handle_t HTTP_Start(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8192;
    config.max_uri_handlers = 12;
    config.lru_purge_enable = true;
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) != ESP_OK) return NULL;
    return server;
}

esp_err_t HTTP_Stop(httpd_handle_t server)
{
    return server ? httpd_stop(server) : ESP_ERR_INVALID_ARG;
}

esp_err_t HTTP_RegisterRoot(httpd_handle_t server,
                            esp_err_t (*handler)(httpd_req_t *req))
{
    return HTTP_RegisterUri(server, "/", HTTP_GET, handler);
}

esp_err_t HTTP_RegisterUri(httpd_handle_t server, const char *uri, httpd_method_t method,
                           esp_err_t (*handler)(httpd_req_t *req))
{
    if (!server || !uri || !handler) return ESP_ERR_INVALID_ARG;
    httpd_uri_t route = {
        .uri = uri,
        .method = method,
        .handler = handler,
        .user_ctx = NULL,
    };
    return httpd_register_uri_handler(server, &route);
}
