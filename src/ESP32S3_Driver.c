#include "ESP32S3_Driver.h"
#include <stdio.h>

void ESP32S3_PrintChipInfo(void)
{
    esp_chip_info_t info = {0};
    esp_chip_info(&info);

    printf("\r\n=== ESP32-S3-DevKitC-1 v1.1 ===\r\n");
    printf("Cores: %d\r\n", info.cores);
    printf("Revision: %d\r\n", info.revision);
    printf("Wi-Fi: yes\r\n");
    printf("Bluetooth LE: yes\r\n");
    printf("Bluetooth Classic: no\r\n");
    printf("TWAI controller: yes\r\n");
}
