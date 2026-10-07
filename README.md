# ESP32-S3-DevKitC-1 v1.1 — All Protocol Drivers

A register/driver-oriented ESP32-S3 project organized in the same conceptual
style as the STM32 driver repository: one header/source pair per hardware or
protocol function plus a central `main.c` example.

## Target

**ESP32-S3-DevKitC-1 v1.1**.

Espressif's current board guide identifies GPIO38 as the v1.1 addressable RGB
LED output, GPIO43/44 as U0TXD/U0RXD on the USB-to-UART bridge, and GPIO20/19
as USB D+/D-. GPIO35/36/37 can be consumed by Octal flash/PSRAM modules, so
avoid them when the fitted module requires them internally.

## Supported

```text
Peripherals
├── GPIO
├── UART
├── SPI
├── I2C
├── ADC
└── TWAI / Classical CAN

Wi-Fi / IP
├── STA
├── AP
├── STA+AP
├── Scan
├── ESP-NOW
├── TCP
├── UDP
├── HTTP server
└── MQTT

Bluetooth
├── BLE controller/host
├── GAP / Advertising
└── GATT server foundation
```

## Bluetooth limitation

ESP32-S3 is **BLE-only**. Bluetooth Classic is not available on this chip.
ESP-IDF supports Bluedroid and NimBLE host stacks, but a BLE-only chip cannot
provide Classic Bluetooth by software. This repository therefore implements
BLE, not Classic SPP/A2DP/etc.

## Startup and linker

There is intentionally no custom `startup.s` or `linker.ld` here. ESP-IDF owns
the bootloader/startup/runtime/linker integration for ESP32-S3.

Use:

```text
app_main()
CMakeLists.txt
main/CMakeLists.txt
sdkconfig.defaults
```

## Example pin map

Board-specific:

```text
RGB LED       GPIO38
USB D+        GPIO20
USB D-        GPIO19
USB-UART TX   GPIO43
USB-UART RX   GPIO44
```

Example application pins:

```text
SPI2 SCLK     GPIO12
SPI2 MOSI     GPIO11
SPI2 MISO     GPIO13
SPI2 CS       GPIO14
I2C0 SDA      GPIO8
I2C0 SCL      GPIO9
ADC1_CH3      GPIO4
TWAI TX       GPIO15
TWAI RX       GPIO16
External LED  GPIO2
```

Verify application pins for your exact module and wiring.

## Build

```bash
. $HOME/esp/esp-idf/export.sh
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

## TWAI / CAN

ESP32-S3 has one TWAI controller supporting Classical CAN frame formats with
11-bit standard and 29-bit extended identifiers. It does not support CAN FD.
An external physical-layer CAN transceiver is required for a normal CAN bus.

## I2C

ESP32-S3 has two I2C controllers. The new ESP-IDF I2C master API is used.
Use external pull-ups appropriate for the bus and rise time.

## UART

ESP32-S3 has three UART controllers. The example uses UART1 on GPIO43/44 so
the DevKitC-1 USB-to-UART bridge can be used for communication.

## SPI

SPI2 and SPI3 are general-purpose user SPI controllers. SPI0/SPI1 are reserved
for internal flash/PSRAM use in normal applications.

## ADC

ESP32-S3 has two ADC units. The sample reads ADC1 channel 3 on GPIO4.

## Wi-Fi

Set:

```c
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

The sample starts STA mode and an HTTP server.

## ESP-NOW / TCP / UDP / HTTP / MQTT

These are separate wrappers so the application can use the appropriate layer
without putting every network feature into one giant Wi-Fi structure.

## BLE

The BLE module creates a small sample GATT service and configures advertising.
Use a BLE scanner to verify the device.
