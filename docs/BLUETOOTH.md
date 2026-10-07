# Bluetooth LE

ESP32-S3 supports Bluetooth LE and does not support Bluetooth Classic.

This project uses ESP-Bluedroid for the BLE controller/host interfaces.

Included foundation:

```text
BLE_Init()
BLE_StartGATTServer()
BLE_StartAdvertising()

GATT service: 0x00FF
RX characteristic: 0xFF01
TX characteristic: 0xFF02
```

Expand the BLE module for notifications, indications, security/SMP, GATT
client, connection management and BLE Mesh as needed.
