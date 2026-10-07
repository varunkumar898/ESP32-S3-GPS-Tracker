#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H
#include "driver/spi_master.h"

typedef struct {
    spi_host_device_t Host;
    int SCLKPin;
    int MOSIPin;
    int MISOPin;
    int CSPin;
    int ClockHz;
    int Mode;
    int QueueSize;
} SPI_InitTypeDef;

esp_err_t SPI_Init(const SPI_InitTypeDef *config, spi_device_handle_t *device);
esp_err_t SPI_Transfer(const uint8_t *tx, uint8_t *rx, size_t length,
                       spi_device_handle_t device);

#endif
