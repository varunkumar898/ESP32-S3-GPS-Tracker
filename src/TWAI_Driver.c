#include "TWAI_Driver.h"

esp_err_t TWAI_Init(const TWAI_InitTypeDef *c, twai_node_handle_t *node)
{
    if (!c || !node) return ESP_ERR_INVALID_ARG;

    twai_onchip_node_config_t cfg = {
        .io_cfg.tx = c->TXPin,
        .io_cfg.rx = c->RXPin,
        .bit_timing.bitrate = c->Bitrate,
        .tx_queue_depth = c->TXQueueDepth,
    };

    cfg.flags.enable_loopback = c->Loopback;
    cfg.flags.enable_listen_only = c->ListenOnly;

    return twai_new_node_onchip(&cfg, node);
}

esp_err_t TWAI_Start(twai_node_handle_t node)
{
    return node ? twai_node_enable(node) : ESP_ERR_INVALID_ARG;
}

esp_err_t TWAI_Stop(twai_node_handle_t node)
{
    return node ? twai_node_disable(node) : ESP_ERR_INVALID_ARG;
}

esp_err_t TWAI_Send(twai_node_handle_t node, uint32_t id,
                    const uint8_t *data, size_t length, bool extended)
{
    if (!node || !data || length > 8U) return ESP_ERR_INVALID_ARG;
    if ((!extended && id > 0x7FFU) || (extended && id > 0x1FFFFFFFU))
        return ESP_ERR_INVALID_ARG;

    twai_frame_t frame = {
        .header.id = id,
        .header.ide = extended,
        .buffer = (uint8_t *)data,
        .buffer_len = length,
    };

    return twai_node_transmit(node, &frame, 0);
}

esp_err_t TWAI_WaitTxDone(twai_node_handle_t node, int timeout_ms)
{
    return node ? twai_node_transmit_wait_all_done(node, timeout_ms)
                : ESP_ERR_INVALID_ARG;
}
