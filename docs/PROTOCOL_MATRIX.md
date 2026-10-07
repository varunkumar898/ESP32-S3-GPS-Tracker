# Protocol Matrix

| Module | ESP32-S3 | Repository driver |
|---|---|---|
| GPIO | Yes | GPIO_Driver |
| UART | 3 controllers | UART_Driver |
| SPI | SPI2/SPI3 general-purpose | SPI_Driver |
| I2C | 2 controllers | I2C_Driver |
| ADC | 2 ADC units | ADC_Driver |
| TWAI / Classical CAN | 1 controller | TWAI_Driver |
| Wi-Fi STA | Yes | WIFI_Driver |
| Wi-Fi AP | Yes | WIFI_Driver |
| Wi-Fi STA+AP | Yes | WIFI_Driver |
| Wi-Fi Scan | Yes | WIFI_Driver |
| ESP-NOW | Yes | ESPNOW_Driver |
| TCP | Yes | TCP_Driver |
| UDP | Yes | UDP_Driver |
| HTTP server | Yes | HTTP_Driver |
| MQTT | Yes | MQTT_Driver |
| BLE GAP | Yes | BLE_Driver |
| BLE GATT | Yes | BLE_Driver |
| BLE SMP/security | Stack support; add profile module | Future |
| BLE Mesh | ESP-IDF support | Future separate module |
| Bluetooth Classic | **No** | Not available on S3 |
| USB OTG | Yes | Separate module recommended |
