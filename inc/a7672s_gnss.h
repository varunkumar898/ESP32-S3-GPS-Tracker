#ifndef A7672S_GNSS_H
#define A7672S_GNSS_H

#include <stdint.h>
#include <stdbool.h>
#include "a7672s.h"

// Telemetry structure storing converted decimal GNSS data
typedef struct {
    bool is_valid;
    char fix_mode[12];     // "No Fix", "2D Fix", "3D Fix"
    int gps_sats;
    int glonass_sats;
    double latitude;       // Decimal Degrees (e.g., 13.0827)
    char ns;               // 'N' or 'S'
    double longitude;      // Decimal Degrees (e.g., 80.2707)
    char ew;               // 'E' or 'W'
    char date[12];         // DDMMYY format
    char time[12];         // HHMMSS.SS format
    float altitude;        // meters
    float speed;           // knots
    float course;          // degrees
    float pdop;
    float hdop;
    float vdop;
} a7672s_gnss_data_t;

// Driver API Prototypes
a7672s_status_t a7672s_gnss_enable(bool enable);
a7672s_status_t a7672s_gnss_get_data(a7672s_gnss_data_t *gps);
bool a7672s_parse_cgnssinfo(const char *response, a7672s_gnss_data_t *gps);
double a7672s_nmea_to_decimal(double raw_coord);

#endif // A7672S_GNSS_H