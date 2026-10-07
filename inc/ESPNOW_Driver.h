#ifndef ESPNOW_DRIVER_H
#define ESPNOW_DRIVER_H
#include "esp_now.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t PeerAddress[ESP_NOW_ETH_ALEN];
    bool Encrypt;
    uint8_t Channel;
} ESPNOW_PeerConfigTypeDef;

esp_err_t ESPNOW_Init(void);
esp_err_t ESPNOW_AddPeer(const ESPNOW_PeerConfigTypeDef *config, const uint8_t *key16);
esp_err_t ESPNOW_Send(const uint8_t *peer, const uint8_t *data, size_t len);
esp_err_t ESPNOW_RegisterReceive(esp_now_recv_cb_t callback);

#endif
