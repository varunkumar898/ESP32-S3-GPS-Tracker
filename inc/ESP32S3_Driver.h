#ifndef ESP32S3_DRIVER_H
#define ESP32S3_DRIVER_H

#include "esp_err.h"
#include "esp_chip_info.h"

#define ESP32S3_WIFI_SUPPORTED       1
#define ESP32S3_BLE_SUPPORTED        1
#define ESP32S3_BT_CLASSIC_SUPPORTED 0
#define ESP32S3_TWAI_SUPPORTED       1

void ESP32S3_PrintChipInfo(void);

#endif
