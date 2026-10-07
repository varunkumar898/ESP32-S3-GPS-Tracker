#include "UART_Driver.h"

esp_err_t UART_Init(const UART_InitTypeDef *c)
{
    if (!c) return ESP_ERR_INVALID_ARG;
    uart_config_t cfg = {
        .baud_rate = c->BaudRate,
        .data_bits = c->WordLength,
        .parity = c->Parity,
        .stop_bits = c->StopBits,
        .flow_ctrl = c->FlowControl,
        .source_clk = UART_SCLK_DEFAULT,
    };
    esp_err_t err = uart_driver_install(c->Port, c->RXBufferSize, c->TXBufferSize, 0, NULL, 0);
    if (err != ESP_OK) return err;
    err = uart_param_config(c->Port, &cfg);
    if (err != ESP_OK) return err;
    return uart_set_pin(c->Port, c->TXPin, c->RXPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int UART_Write(uart_port_t port, const uint8_t *data, size_t len)
{
    if (!data) return -1;
    return uart_write_bytes(port, data, len);
}

int UART_Read(uart_port_t port, uint8_t *data, size_t len, uint32_t timeout_ms)
{
    if (!data) return -1;
    return uart_read_bytes(port, data, len, pdMS_TO_TICKS(timeout_ms));
}

int UART_ReadByte(uart_port_t port, uint8_t *data, uint32_t timeout_ms)
{
    return UART_Read(port, data, 1, timeout_ms) == 1 ? 1 : 0;
}
