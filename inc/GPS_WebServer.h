#ifndef GPS_WEBSERVER_H
#define GPS_WEBSERVER_H

#include "esp_err.h"
#include "GPS_Driver.h"

// Wi-Fi Access Point Configuration
#define GPS_WIFI_AP_SSID        "ESP32-GPS-TRACKER"
#define GPS_WIFI_AP_PASSWORD    "12345678"          // Minimum 8 characters (or empty "" for open)
#define GPS_WIFI_AP_CHANNEL     1
#define GPS_WIFI_AP_MAX_CONN    4

// Optional Wi-Fi Station Configuration (Connects to existing home/mobile hotspot Wi-Fi if enabled)
#define GPS_WIFI_STA_ENABLE     1
#define GPS_WIFI_STA_SSID       "Airtel_HEJAAZ"
#define GPS_WIFI_STA_PASSWORD   "Wifipassword"

/**
 * @brief Initialize Wi-Fi and start the GPS Web Server
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t GPS_WebServer_Start(void);

/**
 * @brief Thread-safe update of the latest GPS telemetry data for the web interface
 * @param data Pointer to the new GPS data struct
 */
void GPS_WebServer_UpdateData(const gps_data_t *data);

/**
 * @brief Thread-safe getter for the latest GPS telemetry data
 * @param out_data Output destination pointer
 */
void GPS_WebServer_GetData(gps_data_t *out_data);

#endif // GPS_WEBSERVER_H
