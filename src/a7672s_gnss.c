#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "a7672s_gnss.h"

// Converts NMEA DDMM.MMMM format to Decimal Degrees (DD.DDDDDD)
double a7672s_nmea_to_decimal(double raw_coord) {
    if (raw_coord == 0.0) return 0.0;
    if (raw_coord < 180.0 && raw_coord > -180.0) return raw_coord;

    int degrees = (int)(raw_coord / 100.0);
    double minutes = raw_coord - (degrees * 100.0);
    return (double)degrees + (minutes / 60.0);
}

// Enable/Disable the GNSS hardware engine inside A7672S
a7672s_status_t a7672s_gnss_enable(bool enable) {
    const char *cmd = enable ? "AT+CGNSSPWR=1\r\n" : "AT+CGNSSPWR=0\r\n";
    return a7672s_send_at_cmd(cmd, "OK", NULL, 0, 2000);
}

// Parse comma-separated AT+CGNSSINFO response safely without strtok (handles empty fields)
bool a7672s_parse_cgnssinfo(const char *response, a7672s_gnss_data_t *gps) {
    if (!response || !gps) return false;

    const char *start = strstr(response, "+CGNSSINFO:");
    if (!start) return false;

    start += 11; // Skip "+CGNSSINFO:"
    while (*start == ' ') start++;

    char buffer[256];
    strncpy(buffer, start, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    // Remove line endings
    char *cr = strchr(buffer, '\r');
    if (cr) *cr = '\0';
    char *lf = strchr(buffer, '\n');
    if (lf) *lf = '\0';

    // Parse up to 16 CSV fields preserving empty values
    char tokens[16][32] = {{0}};
    int token_count = 0;
    const char *ptr = buffer;

    while (*ptr != '\0' && token_count < 16) {
        const char *next_comma = strchr(ptr, ',');
        if (next_comma) {
            int len = next_comma - ptr;
            if (len > 31) len = 31;
            strncpy(tokens[token_count], ptr, len);
            tokens[token_count][len] = '\0';
            token_count++;
            ptr = next_comma + 1;
        } else {
            strncpy(tokens[token_count], ptr, 31);
            tokens[token_count][31] = '\0';
            token_count++;
            break;
        }
    }

    if (token_count < 10) {
        gps->is_valid = false;
        return false;
    }

    int mode = atoi(tokens[0]);
    if (mode == 2) {
        strncpy(gps->fix_mode, "2D Fix", sizeof(gps->fix_mode));
        gps->is_valid = true;
    } else if (mode == 3) {
        strncpy(gps->fix_mode, "3D Fix", sizeof(gps->fix_mode));
        gps->is_valid = true;
    } else {
        strncpy(gps->fix_mode, "No Fix", sizeof(gps->fix_mode));
        gps->is_valid = false;
        return false;
    }

    gps->gps_sats = atoi(tokens[1]);
    gps->glonass_sats = atoi(tokens[2]);

    double raw_lat = atof(tokens[3]);
    gps->ns = (tokens[4][0] != '\0') ? tokens[4][0] : 'N';
    gps->latitude = a7672s_nmea_to_decimal(raw_lat);
    if (gps->ns == 'S') gps->latitude = -gps->latitude;

    double raw_lon = atof(tokens[5]);
    gps->ew = (tokens[6][0] != '\0') ? tokens[6][0] : 'E';
    gps->longitude = a7672s_nmea_to_decimal(raw_lon);
    if (gps->ew == 'W') gps->longitude = -gps->longitude;

    strncpy(gps->date, tokens[7], sizeof(gps->date) - 1);
    strncpy(gps->time, tokens[8], sizeof(gps->time) - 1);
    gps->altitude = atof(tokens[9]);
    if (token_count > 10) gps->speed = atof(tokens[10]);
    if (token_count > 11) gps->course = atof(tokens[11]);
    if (token_count > 12) gps->pdop = atof(tokens[12]);
    if (token_count > 13) gps->hdop = atof(tokens[13]);
    if (token_count > 14) gps->vdop = atof(tokens[14]);

    return true;
}

// Request and parse raw GNSS telemetry over UART
a7672s_status_t a7672s_gnss_get_data(a7672s_gnss_data_t *gps) {
    char rx_buf[384] = {0};
    a7672s_status_t status = a7672s_send_at_cmd("AT+CGNSSINFO\r\n", "OK", rx_buf, sizeof(rx_buf), 2000);

    if (status == A7672S_OK) {
        if (!a7672s_parse_cgnssinfo(rx_buf, gps)) {
            return A7672S_ERROR;
        }
    }
    return status;
}