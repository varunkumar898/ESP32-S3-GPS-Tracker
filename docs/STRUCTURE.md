# Structure

```text
ESP32-S3-DevKitC-1-v1.1-All-Protocol-Drivers/
├── inc/
│   ├── ESP32S3_Driver.h
│   ├── GPIO_Driver.h
│   ├── UART_Driver.h
│   ├── SPI_Driver.h
│   ├── I2C_Driver.h
│   ├── ADC_Driver.h
│   ├── TWAI_Driver.h
│   ├── WIFI_Driver.h
│   ├── ESPNOW_Driver.h
│   ├── TCP_Driver.h
│   ├── UDP_Driver.h
│   ├── HTTP_Driver.h
│   ├── MQTT_Driver.h
│   └── BLE_Driver.h
├── src/
│   ├── main.c
│   └── one source file per driver
├── main/CMakeLists.txt
├── docs/
├── CMakeLists.txt
├── sdkconfig.defaults
├── README.md
└── .gitignore
```

ESP-IDF owns startup, interrupts, linker integration and FreeRTOS runtime;
there is intentionally no hand-written startup.s or linker.ld.
