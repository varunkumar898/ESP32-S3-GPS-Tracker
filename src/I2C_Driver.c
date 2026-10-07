#include "I2C_Driver.h"

esp_err_t I2C_Init(const I2C_InitTypeDef *c, i2c_master_bus_handle_t *bus)
{
    if (!c || !bus) return ESP_ERR_INVALID_ARG;
    i2c_master_bus_config_t cfg = {
        .i2c_port = c->Port,
        .sda_io_num = c->SDAPin,
        .scl_io_num = c->SCLPin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = c->InternalPullups,
    };
    return i2c_new_master_bus(&cfg, bus);
}

esp_err_t I2C_AddDevice(i2c_master_bus_handle_t bus, uint8_t address,
                        uint32_t frequency_hz, i2c_master_dev_handle_t *device)
{
    if (!bus || !device) return ESP_ERR_INVALID_ARG;
    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = frequency_hz,
        .flags.disable_ack_check = false,
    };
    return i2c_master_bus_add_device(bus, &cfg, device);
}

esp_err_t I2C_Write(i2c_master_dev_handle_t device, const uint8_t *data,
                    size_t len, int timeout_ms)
{
    return i2c_master_transmit(device, data, len, timeout_ms);
}

esp_err_t I2C_Read(i2c_master_dev_handle_t device, uint8_t *data,
                   size_t len, int timeout_ms)
{
    return i2c_master_receive(device, data, len, timeout_ms);
}

esp_err_t I2C_WriteRead(i2c_master_dev_handle_t device, const uint8_t *tx,
                        size_t tx_len, uint8_t *rx, size_t rx_len,
                        int timeout_ms)
{
    return i2c_master_transmit_receive(device, tx, tx_len, rx, rx_len, timeout_ms);
}
