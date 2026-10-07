#ifndef UART_DRIVER_H
#define UART_DRIVER_H
#include "driver/uart.h"

typedef struct {
    uart_port_t Port;
    int TXPin;
    int RXPin;
    int BaudRate;
    uart_word_length_t WordLength;
    uart_parity_t Parity;
    uart_stop_bits_t StopBits;
    uart_hw_flowcontrol_t FlowControl;
    int RXBufferSize;
    int TXBufferSize;
} UART_InitTypeDef;

esp_err_t UART_Init(const UART_InitTypeDef *config);
int UART_Write(uart_port_t port, const uint8_t *data, size_t len);
int UART_Read(uart_port_t port, uint8_t *data, size_t len, uint32_t timeout_ms);
int UART_ReadByte(uart_port_t port, uint8_t *data, uint32_t timeout_ms);

#endif
