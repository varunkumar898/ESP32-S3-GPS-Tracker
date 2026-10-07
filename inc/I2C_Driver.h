#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H
#include <stdbool.h>
#include "driver/i2c_master.h"

typedef struct {
    i2c_port_t Port;
    int SCLPin;
    int SDAPin;
    uint32_t FrequencyHz;
    bool InternalPullups;
} I2C_InitTypeDef;

esp_err_t I2C_Init(const I2C_InitTypeDef *config, i2c_master_bus_handle_t *bus);
esp_err_t I2C_AddDevice(i2c_master_bus_handle_t bus, uint8_t address,
                        uint32_t frequency_hz, i2c_master_dev_handle_t *device);
esp_err_t I2C_Write(i2c_master_dev_handle_t device, const uint8_t *data,
                    size_t len, int timeout_ms);
esp_err_t I2C_Read(i2c_master_dev_handle_t device, uint8_t *data,
                   size_t len, int timeout_ms);
esp_err_t I2C_WriteRead(i2c_master_dev_handle_t device, const uint8_t *tx,
                        size_t tx_len, uint8_t *rx, size_t rx_len,
                        int timeout_ms);

#endif
