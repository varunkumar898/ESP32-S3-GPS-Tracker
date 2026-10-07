#include "GPS_Driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "GPS_DRIVER";

// Converts NMEA DDMM.MMMM or DDDMM.MMMM format to Decimal Degrees (DD.DDDDDD)
double GPS_NmeaToDecimal(double raw_coord) {
    if (raw_coord == 0.0) return 0.0;
    
    int degrees = (int)(raw_coord / 100.0);
    double minutes = raw_coord - (degrees * 100.0);
    return (double)degrees + (minutes / 60.0);
}

esp_err_t GPS_Init(const gps_config_t *config) {
    uart_port_t port = config ? config->uart_port : GPS_UART_PORT;
    int rx_pin = config ? config->rx_pin : GPS_DEFAULT_RX_PIN;
    int tx_pin = config ? config->tx_pin : GPS_DEFAULT_TX_PIN;
    int baud = config ? config->baud_rate : GPS_DEFAULT_BAUD;

    uart_config_t uart_cfg = {
        .baud_rate = baud,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    esp_err_t err = uart_driver_install(port, GPS_RX_BUFFER_SIZE, 0, 0, NULL, 0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to install UART driver on port %d: %s", port, esp_err_to_name(err));
        return err;
    }

    err = uart_param_config(port, &uart_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure UART parameters: %s", esp_err_to_name(err));
        return err;
    }

    err = uart_set_pin(port, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART pins TX=%d, RX=%d: %s", tx_pin, rx_pin, esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "GPS UART initialized on Port %d (TX: GPIO %d, RX: GPIO %d, Baud: %d)",
             port, tx_pin, rx_pin, baud);
    return ESP_OK;
}

// Reads bytes until a newline '\n' is encountered or buffer is full / timeout
bool GPS_ReadSentence(uart_port_t port, char *buffer, size_t max_len, uint32_t timeout_ms) {
    if (!buffer || max_len == 0) return false;

    size_t idx = 0;
    TickType_t start_tick = xTaskGetTickCount();
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

    while (idx < max_len - 1) {
        uint8_t ch = 0;
        int len = uart_read_bytes(port, &ch, 1, pdMS_TO_TICKS(20));
        if (len > 0) {
            if (ch == '$') {
                idx = 0; // Restart from beginning of NMEA sentence
                buffer[idx++] = ch;
            } else if (idx > 0) {
                if (ch == '\n' || ch == '\r') {
                    if (idx > 0) {
                        buffer[idx] = '\0';
                        return true;
                    }
                } else {
                    buffer[idx++] = ch;
                }
            }
        }

        if ((xTaskGetTickCount() - start_tick) > timeout_ticks) {
            break;
        }
    }

    buffer[idx] = '\0';
    return false;
}

// Helper to extract CSV tokens handling empty slots
static int extract_tokens(const char *str, char tokens[][32], int max_tokens) {
    int count = 0;
    const char *ptr = str;

    while (*ptr != '\0' && count < max_tokens) {
        const char *comma = strchr(ptr, ',');
        const char *star = strchr(ptr, '*');
        const char *delim = comma;
        if (!delim || (star && star < delim)) {
            delim = star;
        }

        if (delim) {
            int len = delim - ptr;
            if (len > 31) len = 31;
            strncpy(tokens[count], ptr, len);
            tokens[count][len] = '\0';
            count++;
            ptr = delim + 1;
        } else {
            strncpy(tokens[count], ptr, 31);
            tokens[count][31] = '\0';
            count++;
            break;
        }
    }
    return count;
}

// Parses NMEA sentence ($GPRMC, $GNRMC, $GPGGA, $GNGGA)
bool GPS_ParseNMEA(const char *sentence, gps_data_t *gps) {
    if (!sentence || !gps || sentence[0] != '$') return false;

    char tokens[20][32] = {{0}};
    int token_count = extract_tokens(sentence, tokens, 20);
    if (token_count < 3) return false;

    const char *type = tokens[0];

    // Check for RMC sentences: $GPRMC, $GNRMC, $BDRMC, $GARMC
    if (strstr(type, "RMC")) {
        // Format: $xxRMC,time,status,lat,N/S,lon,E/W,speed_knots,course,date,...
        // Status: 'A' = Valid fix, 'V' = Void (no fix)
        char status = tokens[2][0];
        if (status == 'A') {
            gps->is_valid = true;

            // UTC Time (HHMMSS.SS)
            if (strlen(tokens[1]) >= 6) {
                snprintf(gps->time, sizeof(gps->time), "%c%c:%c%c:%c%c UTC",
                         tokens[1][0], tokens[1][1], tokens[1][2], tokens[1][3], tokens[1][4], tokens[1][5]);
            }

            // Latitude
            double raw_lat = atof(tokens[3]);
            gps->lat_direction = tokens[4][0] ? tokens[4][0] : 'N';
            gps->latitude = GPS_NmeaToDecimal(raw_lat);
            if (gps->lat_direction == 'S') gps->latitude = -gps->latitude;

            // Longitude
            double raw_lon = atof(tokens[5]);
            gps->lon_direction = tokens[6][0] ? tokens[6][0] : 'E';
            gps->longitude = GPS_NmeaToDecimal(raw_lon);
            if (gps->lon_direction == 'W') gps->longitude = -gps->longitude;

            // Speed in knots -> converted to km/h
            float speed_knots = atof(tokens[7]);
            gps->speed_kmh = speed_knots * 1.852f;

            // Course in degrees
            gps->course_deg = atof(tokens[8]);

            // Date (DDMMYY)
            if (strlen(tokens[9]) >= 6) {
                snprintf(gps->date, sizeof(gps->date), "%c%c/%c%c/20%c%c",
                         tokens[9][0], tokens[9][1], tokens[9][2], tokens[9][3], tokens[9][4], tokens[9][5]);
            }

            return true;
        } else {
            gps->is_valid = false;
            return false;
        }
    }
    // Check for GGA sentences: $GPGGA, $GNGGA, $BDGGA, $GAGGA
    else if (strstr(type, "GGA")) {
        // Format: $xxGGA,time,lat,N/S,lon,E/W,quality,sats,hdop,alt,M,height,M,...
        int fix_qual = atoi(tokens[6]);
        gps->fix_quality = fix_qual;
        gps->satellites = atoi(tokens[7]);

        if (fix_qual > 0) {
            gps->is_valid = true;

            // Latitude
            double raw_lat = atof(tokens[2]);
            gps->lat_direction = tokens[3][0] ? tokens[3][0] : 'N';
            gps->latitude = GPS_NmeaToDecimal(raw_lat);
            if (gps->lat_direction == 'S') gps->latitude = -gps->latitude;

            // Longitude
            double raw_lon = atof(tokens[4]);
            gps->lon_direction = tokens[5][0] ? tokens[5][0] : 'E';
            gps->longitude = GPS_NmeaToDecimal(raw_lon);
            if (gps->lon_direction == 'W') gps->longitude = -gps->longitude;

            // Altitude
            gps->altitude = atof(tokens[9]);

            return true;
        } else {
            gps->is_valid = false;
            return false;
        }
    }

    return false;
}

// Formats and prints location in terminal
void GPS_PrintLocation(const gps_data_t *gps) {
    if (!gps) return;

    if (gps->is_valid) {
        printf("\r\n================ GPS LOCATION FIX ================\r\n");
        printf(" Latitude   : %11.6f deg %c\r\n", (gps->latitude >= 0 ? gps->latitude : -gps->latitude), gps->lat_direction);
        printf(" Longitude  : %11.6f deg %c\r\n", (gps->longitude >= 0 ? gps->longitude : -gps->longitude), gps->lon_direction);
        printf(" Full Format: %11.6f, %11.6f\r\n", gps->latitude, gps->longitude);
        printf(" Altitude   : %.2f m\r\n", gps->altitude);
        printf(" Speed      : %.2f km/h\r\n", gps->speed_kmh);
        printf(" Satellites : %d\r\n", gps->satellites);
        if (gps->date[0] != '\0') printf(" Date       : %s\r\n", gps->date);
        if (gps->time[0] != '\0') printf(" Time       : %s\r\n", gps->time);
        printf(" Google Maps: https://www.google.com/maps?q=%.6f,%.6f\r\n", gps->latitude, gps->longitude);
        printf("==================================================\r\n");
    } else {
        printf("[GPS] Searching for satellites (No Fix)... Satellites in view: %d\r\n", gps->satellites);
    }
}
