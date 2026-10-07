#ifndef GPS_DRIVER_H
#define GPS_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/uart.h"
#include "esp_err.h"

// Default Pin assignments specified by user
#define GPS_UART_PORT       UART_NUM_1
#define GPS_DEFAULT_RX_PIN  17
#define GPS_DEFAULT_TX_PIN  18
#define GPS_DEFAULT_BAUD    9600
#define GPS_RX_BUFFER_SIZE  1024

typedef struct {
    bool is_valid;          // true when GPS has a valid fix
    int fix_quality;        // 0: Invalid, 1: GPS Fix, 2: DGPS Fix
    int satellites;         // Number of satellites tracked
    double latitude;        // Decimal Degrees (e.g. 13.082680)
    char lat_direction;     // 'N' or 'S'
    double longitude;       // Decimal Degrees (e.g. 80.270718)
    char lon_direction;     // 'E' or 'W'
    float altitude;         // Altitude in meters above sea level
    float speed_kmh;        // Speed in km/h
    float course_deg;       // Course / heading in degrees
    char time[16];          // UTC Time (HH:MM:SS)
    char date[16];          // UTC Date (DD/MM/YYYY)
} gps_data_t;

typedef struct {
    uart_port_t uart_port;
    int rx_pin;
    int tx_pin;
    int baud_rate;
} gps_config_t;

/**
 * @brief Initialize UART interface for GPS Module
 * @param config Pointer to GPS configuration structure (NULL for defaults)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t GPS_Init(const gps_config_t *config);

/**
 * @brief Reads a single NMEA sentence line from the GPS UART stream
 * @param port UART port
 * @param buffer Output buffer for the sentence
 * @param max_len Maximum length of the buffer
 * @param timeout_ms Timeout in milliseconds
 * @return true if a complete sentence was read, false on timeout/error
 */
bool GPS_ReadSentence(uart_port_t port, char *buffer, size_t max_len, uint32_t timeout_ms);

/**
 * @brief Parses an NMEA sentence ($GPRMC, $GNRMC, $GPGGA, $GNGGA, etc.)
 * @param nmea_sentence Raw sentence string starting with '$'
 * @param gps_data Output data structure updated with parsed fields
 * @return true if sentence was recognized and parsed, false otherwise
 */
bool GPS_ParseNMEA(const char *nmea_sentence, gps_data_t *gps_data);

/**
 * @brief Converts NMEA DDMM.MMMM coordinate format to Decimal Degrees (DD.DDDDDD)
 * @param raw_coord Raw coordinate from NMEA sentence
 * @return Decimal degree value
 */
double GPS_NmeaToDecimal(double raw_coord);

/**
 * @brief Formats and prints GPS location and status nicely to terminal
 * @param gps_data Pointer to gps_data_t structure
 */
void GPS_PrintLocation(const gps_data_t *gps_data);

#endif // GPS_DRIVER_H
