#include "SPI_Driver.h"

esp_err_t SPI_Init(const SPI_InitTypeDef *c, spi_device_handle_t *device)
{
    if (!c || !device) return ESP_ERR_INVALID_ARG;
    spi_bus_config_t bus = {
        .sclk_io_num = c->SCLKPin,
        .mosi_io_num = c->MOSIPin,
        .miso_io_num = c->MISOPin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    esp_err_t err = spi_bus_initialize(c->Host, &bus, SPI_DMA_CH_AUTO);
    if (err != ESP_OK) return err;
    spi_device_interface_config_t dev = {
        .clock_speed_hz = c->ClockHz,
        .mode = c->Mode,
        .spics_io_num = c->CSPin,
        .queue_size = c->QueueSize,
    };
    return spi_bus_add_device(c->Host, &dev, device);
}

esp_err_t SPI_Transfer(const uint8_t *tx, uint8_t *rx, size_t length,
                       spi_device_handle_t device)
{
    if (!device || !tx || !rx || length == 0) return ESP_ERR_INVALID_ARG;
    spi_transaction_t t = {
        .length = length * 8U,
        .tx_buffer = tx,
        .rx_buffer = rx,
    };
    return spi_device_transmit(device, &t);
}
