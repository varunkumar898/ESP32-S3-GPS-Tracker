#ifndef TWAI_DRIVER_H
#define TWAI_DRIVER_H
#include "driver/gpio.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"

#include <stdbool.h>

typedef struct {
    gpio_num_t TXPin;
    gpio_num_t RXPin;
    uint32_t Bitrate;
    uint32_t TXQueueDepth;
    bool Loopback;
    bool ListenOnly;
} TWAI_InitTypeDef;

esp_err_t TWAI_Init(const TWAI_InitTypeDef *config, twai_node_handle_t *node);
esp_err_t TWAI_Start(twai_node_handle_t node);
esp_err_t TWAI_Stop(twai_node_handle_t node);
esp_err_t TWAI_Send(twai_node_handle_t node, uint32_t id,
                    const uint8_t *data, size_t length, bool extended);
esp_err_t TWAI_WaitTxDone(twai_node_handle_t node, int timeout_ms);

#endif
