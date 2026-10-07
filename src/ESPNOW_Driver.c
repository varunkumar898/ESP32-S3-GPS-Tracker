#include "ESPNOW_Driver.h"
#include <string.h>

static bool s_init = false;

esp_err_t ESPNOW_Init(void)
{
    if (s_init) return ESP_OK;
    esp_err_t ret = esp_now_init();
    if (ret == ESP_OK) s_init = true;
    return ret;
}

esp_err_t ESPNOW_AddPeer(const ESPNOW_PeerConfigTypeDef *c, const uint8_t *key16)
{
    if (!c) return ESP_ERR_INVALID_ARG;
    esp_now_peer_info_t peer = {0};
    memcpy(peer.peer_addr, c->PeerAddress, ESP_NOW_ETH_ALEN);
    peer.channel = c->Channel;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = c->Encrypt;
    if (c->Encrypt && key16) memcpy(peer.lmk, key16, ESP_NOW_KEY_LEN);
    return esp_now_add_peer(&peer);
}

esp_err_t ESPNOW_Send(const uint8_t *peer, const uint8_t *data, size_t len)
{
    if (!peer || !data || len == 0 || len > ESP_NOW_MAX_DATA_LEN) return ESP_ERR_INVALID_ARG;
    return esp_now_send(peer, data, len);
}

esp_err_t ESPNOW_RegisterReceive(esp_now_recv_cb_t callback)
{
    return callback ? esp_now_register_recv_cb(callback) : ESP_ERR_INVALID_ARG;
}
