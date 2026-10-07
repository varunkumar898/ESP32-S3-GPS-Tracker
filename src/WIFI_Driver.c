#include "WIFI_Driver.h"
#include <string.h>
#include <stdio.h>
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

static int s_retry = 0;
static bool s_event_handlers_registered = false;

static void wifi_event_handler(void *arg, esp_event_base_t base,
                               int32_t id, void *event_data)
{
    (void)arg;
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        (void)esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        s_retry++;
        (void)esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        const ip_event_got_ip_t *e = (const ip_event_got_ip_t *)event_data;
        printf("Wi-Fi IP: " IPSTR "\r\n", IP2STR(&e->ip_info.ip));
        s_retry = 0;
    }
}

esp_err_t WIFI_Init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) return ret;

    ret = esp_netif_init();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) return ret;
    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) return ret;

    if (!esp_netif_get_handle_from_ifkey("WIFI_STA_DEF")) esp_netif_create_default_wifi_sta();
    if (!esp_netif_get_handle_from_ifkey("WIFI_AP_DEF")) esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) return ret;

    if (!s_event_handlers_registered) {
        ret = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
        if (ret != ESP_OK) return ret;
        ret = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
        if (ret != ESP_OK) return ret;
        s_event_handlers_registered = true;
    }
    return ESP_OK;
}

esp_err_t WIFI_StartSTA(const WIFI_STA_InitTypeDef *c)
{
    if (!c || !c->SSID || !c->Password) return ESP_ERR_INVALID_ARG;
    wifi_config_t cfg = {0};
    strlcpy((char *)cfg.sta.ssid, c->SSID, sizeof(cfg.sta.ssid));
    strlcpy((char *)cfg.sta.password, c->Password, sizeof(cfg.sta.password));
    cfg.sta.channel = c->Channel;
    s_retry = 0;
    esp_err_t ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) return ret;
    ret = esp_wifi_set_config(WIFI_IF_STA, &cfg);
    if (ret != ESP_OK) return ret;
    return esp_wifi_start();
}

esp_err_t WIFI_StartAP(const WIFI_AP_InitTypeDef *c)
{
    if (!c || !c->SSID) return ESP_ERR_INVALID_ARG;
    wifi_config_t cfg = {0};
    strlcpy((char *)cfg.ap.ssid, c->SSID, sizeof(cfg.ap.ssid));
    cfg.ap.ssid_len = strlen(c->SSID);
    if (c->Password && strlen(c->Password) >= 8) {
        strlcpy((char *)cfg.ap.password, c->Password, sizeof(cfg.ap.password));
        cfg.ap.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
        cfg.ap.authmode = WIFI_AUTH_OPEN;
    }
    cfg.ap.channel = c->Channel ? c->Channel : 1;
    cfg.ap.max_connection = c->MaxConnections ? c->MaxConnections : 4;
    
    esp_err_t ret = esp_wifi_set_mode(WIFI_MODE_AP);
    if (ret != ESP_OK) return ret;
    ret = esp_wifi_set_config(WIFI_IF_AP, &cfg);
    if (ret != ESP_OK) return ret;
    return esp_wifi_start();
}

esp_err_t WIFI_StartSTA_AP(const WIFI_STA_InitTypeDef *sta, const WIFI_AP_InitTypeDef *ap)
{
    if (!sta || !ap || !ap->SSID) return ESP_ERR_INVALID_ARG;
    wifi_config_t s = {0}, a = {0};
    if (sta->SSID) strlcpy((char *)s.sta.ssid, sta->SSID, sizeof(s.sta.ssid));
    if (sta->Password) strlcpy((char *)s.sta.password, sta->Password, sizeof(s.sta.password));
    s.sta.channel = sta->Channel;

    strlcpy((char *)a.ap.ssid, ap->SSID, sizeof(a.ap.ssid));
    a.ap.ssid_len = strlen(ap->SSID);
    if (ap->Password && strlen(ap->Password) >= 8) {
        strlcpy((char *)a.ap.password, ap->Password, sizeof(a.ap.password));
        a.ap.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
        a.ap.authmode = WIFI_AUTH_OPEN;
    }
    a.ap.channel = ap->Channel ? ap->Channel : 1;
    a.ap.max_connection = ap->MaxConnections ? ap->MaxConnections : 4;

    esp_err_t ret = esp_wifi_set_mode(WIFI_MODE_APSTA);
    if (ret != ESP_OK) return ret;
    ret = esp_wifi_set_config(WIFI_IF_STA, &s);
    if (ret != ESP_OK) return ret;
    ret = esp_wifi_set_config(WIFI_IF_AP, &a);
    if (ret != ESP_OK) return ret;
    return esp_wifi_start();
}

esp_err_t WIFI_Scan(void)
{
    wifi_scan_config_t cfg = { .scan_type = WIFI_SCAN_TYPE_ACTIVE, .show_hidden = true };
    esp_err_t ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) return ret;
    ret = esp_wifi_start();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) return ret;
    ret = esp_wifi_scan_start(&cfg, true);
    if (ret != ESP_OK) return ret;
    uint16_t count = 0;
    ret = esp_wifi_scan_get_ap_num(&count);
    if (ret != ESP_OK) return ret;
    wifi_ap_record_t records[20];
    uint16_t n = count > 20 ? 20 : count;
    ret = esp_wifi_scan_get_ap_records(&n, records);
    if (ret != ESP_OK) return ret;
    for (uint16_t i = 0; i < n; ++i)
        printf("%u: %s RSSI=%d CH=%u\r\n", i, (char *)records[i].ssid, records[i].rssi, records[i].primary);
    return ESP_OK;
}
