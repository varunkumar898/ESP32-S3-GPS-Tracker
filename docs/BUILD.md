# Build

```bash
. $HOME/esp/esp-idf/export.sh
cd ESP32-S3-DevKitC-1-v1.1-All-Protocol-Drivers
idf.py set-target esp32s3
idf.py build
```

Flash and monitor:

```bash
idf.py -p /dev/ttyACM0 flash monitor
```

Use the actual port shown by:

```bash
ls /dev/ttyACM* /dev/ttyUSB*
```
