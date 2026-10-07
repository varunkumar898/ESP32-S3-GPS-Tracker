#include "UDP_Driver.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int UDP_Open(uint16_t local_port)
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) return -1;
    struct sockaddr_in addr = { .sin_family = AF_INET,
                                .sin_port = htons(local_port),
                                .sin_addr.s_addr = htonl(INADDR_ANY) };
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock); return -1;
    }
    return sock;
}

int UDP_SendTo(int sock, const char *host, uint16_t port,
               const void *data, size_t len)
{
    if (!host || !data) return -1;
    struct hostent *he = gethostbyname(host);
    if (!he) return -1;
    struct sockaddr_in to = { .sin_family = AF_INET, .sin_port = htons(port) };
    memcpy(&to.sin_addr, he->h_addr_list[0], sizeof(to.sin_addr));
    return sendto(sock, data, len, 0, (struct sockaddr *)&to, sizeof(to));
}

int UDP_Receive(int sock, void *data, size_t len, int timeout_ms)
{
    if (timeout_ms > 0) {
        struct timeval tv = { .tv_sec = timeout_ms / 1000,
                              .tv_usec = (timeout_ms % 1000) * 1000 };
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }
    return recvfrom(sock, data, len, 0, NULL, NULL);
}

void UDP_Close(int sock)
{
    if (sock >= 0) close(sock);
}
