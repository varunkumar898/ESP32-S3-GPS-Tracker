# Wi-Fi / IP Protocols

The Wi-Fi wrapper provides STA, AP, STA+AP and scan functions.

The project also contains:

```text
ESP-NOW
TCP sockets
UDP sockets
HTTP server
MQTT client
```

Set credentials in `src/main.c` before enabling station mode.

For production, move Wi-Fi credentials to NVS/Kconfig rather than source.
