# Register Reference

This repository deliberately does not invent a single STM32-style register
header for ESP32-S3.

For low-level silicon/register study, use Espressif's target-specific headers
under the ESP-IDF `soc/` and `hal/` layers. The protocol drivers in this repo
wrap the supported ESP-IDF APIs.

Board facts for ESP32-S3-DevKitC-1 v1.1:

```text
RGB LED    GPIO38
USB D+     GPIO20
USB D-     GPIO19
U0TXD      GPIO43
U0RXD      GPIO44
```

On Octal flash/PSRAM module variants, GPIO35/36/37 can be occupied internally.
Do not blindly use them as external GPIOs.
