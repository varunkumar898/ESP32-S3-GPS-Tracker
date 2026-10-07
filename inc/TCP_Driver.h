#ifndef TCP_DRIVER_H
#define TCP_DRIVER_H
#include <stdint.h>
#include <stddef.h>
int TCP_Connect(const char *host, uint16_t port, int timeout_ms);
int TCP_Listen(uint16_t port, int backlog);
int TCP_Send(int sock, const void *data, size_t len);
int TCP_Receive(int sock, void *data, size_t len, int timeout_ms);
void TCP_Close(int sock);
#endif
