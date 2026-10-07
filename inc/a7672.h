// inc/a7672s.h
#ifndef A7672S_H
#define A7672S_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    A7672S_OK = 0,
    A7672S_TIMEOUT,
    A7672S_ERROR,
    A7672S_BUSY
} a7672s_status_t;

typedef enum {
    A7672S_NET_UNREGISTERED = 0,
    A7672S_NET_HOME_NETWORK = 1,
    A7672S_NET_SEARCHING    = 2,
    A7672S_NET_DENIED       = 3,
    A7672S_NET_ROAMING      = 5
} a7672s_net_stat_t;

typedef struct {
    int rssi;       // Received Signal Strength Indicator (0-31, 99 = unknown)
    int ber;        // Bit Error Rate
} a7672s_csq_t;

// Driver API Prototypes
a7672s_status_t a7672s_init(void);
a7672s_status_t a7672s_send_at_cmd(const char *cmd, const char *expected_resp, char *out_buf, uint32_t buf_len, uint32_t timeout_ms);
a7672s_status_t a7672s_get_signal_quality(a7672s_csq_t *csq);
a7672s_status_t a7672s_get_network_status(a7672s_net_stat_t *status);

#endif // A7672S_H