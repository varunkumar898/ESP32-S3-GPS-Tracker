#ifndef UDP_DRIVER_H
#define UDP_DRIVER_H
#include <stdint.h>
#include <stddef.h>
int UDP_Open(uint16_t local_port);
int UDP_SendTo(int sock, const char *host, uint16_t port,
               const void *data, size_t len);
int UDP_Receive(int sock, void *data, size_t len, int timeout_ms);
void UDP_Close(int sock);
#endif
