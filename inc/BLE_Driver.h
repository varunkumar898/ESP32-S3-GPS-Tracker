#ifndef BLE_DRIVER_H
#define BLE_DRIVER_H
#include "esp_err.h"

/* ESP32-S3 supports BLE only; Bluetooth Classic is not supported by the chip. */
typedef struct {
    const char *DeviceName;
} BLE_InitTypeDef;

esp_err_t BLE_Init(const BLE_InitTypeDef *config);
esp_err_t BLE_StartGATTServer(void);
esp_err_t BLE_StartAdvertising(void);

#endif
