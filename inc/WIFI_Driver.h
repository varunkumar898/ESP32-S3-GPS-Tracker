#ifndef WIFI_DRIVER_H
#define WIFI_DRIVER_H
#include "esp_wifi.h"

typedef struct {
    const char *SSID;
    const char *Password;
    uint8_t Channel;
    uint8_t MaxRetry;
} WIFI_STA_InitTypeDef;

typedef struct {
    const char *SSID;
    const char *Password;
    uint8_t Channel;
    uint8_t MaxConnections;
} WIFI_AP_InitTypeDef;

esp_err_t WIFI_Init(void);
esp_err_t WIFI_StartSTA(const WIFI_STA_InitTypeDef *config);
esp_err_t WIFI_StartAP(const WIFI_AP_InitTypeDef *config);
esp_err_t WIFI_StartSTA_AP(const WIFI_STA_InitTypeDef *sta,
                           const WIFI_AP_InitTypeDef *ap);
esp_err_t WIFI_Scan(void);

#endif
