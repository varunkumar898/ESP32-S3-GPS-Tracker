#include "GPS_WebServer.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "WIFI_Driver.h"
#include "HTTP_Driver.h"

static const char *TAG = "GPS_WEBSERVER";

static httpd_handle_t s_server = NULL;
static SemaphoreHandle_t s_gps_mutex = NULL;
static gps_data_t s_latest_gps = {0};

// Thread-safe update of GPS telemetry
void GPS_WebServer_UpdateData(const gps_data_t *data) {
    if (!data) return;
    if (s_gps_mutex && xSemaphoreTake(s_gps_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        s_latest_gps = *data;
        xSemaphoreGive(s_gps_mutex);
    }
}

// Thread-safe getter
void GPS_WebServer_GetData(gps_data_t *out_data) {
    if (!out_data) return;
    if (s_gps_mutex && xSemaphoreTake(s_gps_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        *out_data = s_latest_gps;
        xSemaphoreGive(s_gps_mutex);
    }
}

// Embedded Web Application HTML/CSS/JS Dashboard
static const char INDEX_HTML[] = 
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>ESP32-S3 GPS Live Tracker</title>"
"<link rel=\"stylesheet\" href=\"https://unpkg.com/leaflet@1.9.4/dist/leaflet.css\" />"
"<style>"
":root {"
"--bg-primary: #0b0f19;"
"--bg-card: rgba(23, 32, 54, 0.75);"
"--accent: #38bdf8;"
"--accent-glow: rgba(56, 189, 248, 0.25);"
"--text-main: #f8fafc;"
"--text-muted: #94a3b8;"
"--success: #22c55e;"
"--warning: #f59e0b;"
"--danger: #ef4444;"
"--border: rgba(255, 255, 255, 0.08);"
"}"
"* { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; }"
"body { background: var(--bg-primary); color: var(--text-main); min-height: 100vh; padding: 1.25rem; display: flex; flex-direction: column; gap: 1.25rem; }"
".header { display: flex; justify-content: space-between; align-items: center; padding: 1rem 1.5rem; background: var(--bg-card); backdrop-filter: blur(12px); border: 1px solid var(--border); border-radius: 16px; }"
".title-box { display: flex; align-items: center; gap: 0.75rem; }"
".title-box h1 { font-size: 1.25rem; font-weight: 700; color: #fff; letter-spacing: -0.02em; }"
".status-badge { display: flex; align-items: center; gap: 0.5rem; padding: 0.35rem 0.85rem; border-radius: 9999px; font-size: 0.8rem; font-weight: 600; text-transform: uppercase; letter-spacing: 0.05em; background: rgba(245, 158, 11, 0.15); color: var(--warning); border: 1px solid rgba(245, 158, 11, 0.3); }"
".status-badge.fixed { background: rgba(34, 197, 94, 0.15); color: var(--success); border-color: rgba(34, 197, 94, 0.3); }"
".dot { width: 8px; height: 8px; border-radius: 50%; background: currentColor; animation: pulse 1.5s infinite; }"
"@keyframes pulse { 0%, 100% { opacity: 1; transform: scale(1); } 50% { opacity: 0.4; transform: scale(0.85); } }"
".grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 1rem; }"
".card { background: var(--bg-card); backdrop-filter: blur(12px); border: 1px solid var(--border); border-radius: 16px; padding: 1.25rem; display: flex; flex-direction: column; gap: 0.5rem; transition: transform 0.2s ease, border-color 0.2s ease; }"
".card:hover { border-color: var(--accent-glow); transform: translateY(-2px); }"
".card-label { font-size: 0.75rem; font-weight: 600; text-transform: uppercase; color: var(--text-muted); letter-spacing: 0.05em; display: flex; justify-content: space-between; align-items: center; }"
".card-value { font-size: 1.5rem; font-weight: 700; color: #fff; word-break: break-all; font-variant-numeric: tabular-nums; }"
".card-value.highlight { color: var(--accent); text-shadow: 0 0 20px var(--accent-glow); }"
".card-sub { font-size: 0.8rem; color: var(--text-muted); }"
".coords-card { grid-column: 1 / -1; display: flex; flex-direction: column; gap: 1rem; background: linear-gradient(135deg, rgba(30, 41, 59, 0.7), rgba(15, 23, 42, 0.9)); border: 1px solid rgba(56, 189, 248, 0.3); }"
".coords-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 1rem; }"
"@media (max-width: 600px) { .coords-grid { grid-template-columns: 1fr; } }"
".actions { display: flex; gap: 0.75rem; flex-wrap: wrap; margin-top: 0.5rem; }"
".btn { display: inline-flex; align-items: center; justify-content: center; gap: 0.5rem; padding: 0.65rem 1.25rem; border-radius: 10px; font-size: 0.875rem; font-weight: 600; text-decoration: none; cursor: pointer; transition: all 0.2s ease; border: none; }"
".btn-primary { background: #0284c7; color: #fff; box-shadow: 0 4px 14px rgba(2, 132, 199, 0.35); }"
".btn-primary:hover { background: #0369a1; transform: translateY(-1px); }"
".btn-secondary { background: rgba(255, 255, 255, 0.08); color: #fff; border: 1px solid var(--border); }"
".btn-secondary:hover { background: rgba(255, 255, 255, 0.15); }"
"#map-container { height: 420px; width: 100%; border-radius: 16px; overflow: hidden; border: 1px solid var(--border); background: #1e293b; position: relative; }"
"#map { height: 100%; width: 100%; z-index: 1; }"
".map-overlay-msg { position: absolute; inset: 0; display: flex; flex-direction: column; align-items: center; justify-content: center; background: rgba(11, 15, 25, 0.85); z-index: 10; gap: 0.75rem; padding: 1rem; text-align: center; }"
".toast { position: fixed; bottom: 1.5rem; right: 1.5rem; background: var(--success); color: #fff; padding: 0.75rem 1.25rem; border-radius: 10px; font-weight: 600; box-shadow: 0 10px 25px rgba(0,0,0,0.5); opacity: 0; transform: translateY(20px); transition: all 0.3s cubic-bezier(0.16, 1, 0.3, 1); pointer-events: none; z-index: 9999; }"
".toast.show { opacity: 1; transform: translateY(0); }"
"</style>"
"</head>"
"<body>"
"<div class=\"header\">"
"<div class=\"title-box\">"
"<h1>ESP32-S3 GPS Tracker</h1>"
"</div>"
"<div id=\"fix-badge\" class=\"status-badge\">"
"<span class=\"dot\"></span>"
"<span id=\"fix-text\">Searching GPS...</span>"
"</div>"
"</div>"
"<div class=\"grid\">"
"<div class=\"card coords-card\">"
"<div class=\"card-label\"><span>GPS Coordinates (Decimal Degrees)</span><span id=\"gps-time\">--:--:-- UTC</span></div>"
"<div class=\"coords-grid\">"
"<div>"
"<div class=\"card-sub\">Latitude</div>"
"<div id=\"lat-val\" class=\"card-value highlight\">0.000000°</div>"
"</div>"
"<div>"
"<div class=\"card-sub\">Longitude</div>"
"<div id=\"lon-val\" class=\"card-value highlight\">0.000000°</div>"
"</div>"
"</div>"
"<div class=\"actions\">"
"<a id=\"gmaps-btn\" href=\"#\" target=\"_blank\" class=\"btn btn-primary\">📍 Open in Google Maps</a>"
"<button onclick=\"copyCoords()\" class=\"btn btn-secondary\">📋 Copy Coordinates</button>"
"<button onclick=\"recenterMap()\" class=\"btn btn-secondary\">🎯 Center Map</button>"
"</div>"
"</div>"
"<div class=\"card\">"
"<div class=\"card-label\">Satellites</div>"
"<div id=\"sats-val\" class=\"card-value\">0</div>"
"<div class=\"card-sub\">Tracked GNSS satellites</div>"
"</div>"
"<div class=\"card\">"
"<div class=\"card-label\">Altitude</div>"
"<div id=\"alt-val\" class=\"card-value\">0.0 m</div>"
"<div class=\"card-sub\">Above sea level</div>"
"</div>"
"<div class=\"card\">"
"<div class=\"card-label\">Speed</div>"
"<div id=\"speed-val\" class=\"card-value\">0.0 km/h</div>"
"<div class=\"card-sub\">Ground speed</div>"
"</div>"
"<div class=\"card\">"
"<div class=\"card-label\">Date & Fix Quality</div>"
"<div id=\"date-val\" class=\"card-value\">--/--/----</div>"
"<div id=\"qual-val\" class=\"card-sub\">Fix: Standalone</div>"
"</div>"
"</div>"
"<div id=\"map-container\">"
"<div id=\"map\"></div>"
"<div id=\"map-overlay\" class=\"map-overlay-msg\">"
"<h3>🛰️ Waiting for GPS satellite lock...</h3>"
"<p style=\"color: var(--text-muted); font-size: 0.9rem;\">Ensure GPS antenna has clear line-of-sight with sky.</p>"
"</div>"
"</div>"
"<div id=\"toast\" class=\"toast\">Coordinates copied to clipboard!</div>"
"<script src=\"https://unpkg.com/leaflet@1.9.4/dist/leaflet.js\"></script>"
"<script>"
"let map = null, marker = null, hasInitialFix = false;"
"let currentLat = 0, currentLon = 0;"
"function initMap() {"
"if (typeof L === 'undefined') return;"
"map = L.map('map').setView([20, 0], 2);"
"L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {"
"maxZoom: 19,"
"attribution: '© OpenStreetMap'"
"}).addTo(map);"
"}"
"function updateUI(data) {"
"const badge = document.getElementById('fix-badge');"
"const fixText = document.getElementById('fix-text');"
"const mapOverlay = document.getElementById('map-overlay');"
"if (data.valid) {"
"badge.className = 'status-badge fixed';"
"fixText.innerText = data.fix > 1 ? 'DGPS Fix' : '3D GPS Fix';"
"mapOverlay.style.display = 'none';"
"currentLat = data.lat;"
"currentLon = data.lon;"
"document.getElementById('lat-val').innerText = data.lat.toFixed(6) + '° ' + data.lat_dir;"
"document.getElementById('lon-val').innerText = data.lon.toFixed(6) + '° ' + data.lon_dir;"
"document.getElementById('gmaps-btn').href = `https://www.google.com/maps?q=${data.lat},${data.lon}`;"
"if (map) {"
"const latlng = [data.lat, data.lon];"
"if (!marker) {"
"marker = L.marker(latlng).addTo(map);"
"map.setView(latlng, 16);"
"} else {"
"marker.setLatLng(latlng);"
"if (!hasInitialFix) { map.setView(latlng, 16); hasInitialFix = true; }"
"}"
"marker.bindPopup(`<b>ESP32-S3 Location</b><br>Lat: ${data.lat.toFixed(6)}<br>Lon: ${data.lon.toFixed(6)}<br>Alt: ${data.alt}m`).openPopup();"
"}"
"} else {"
"badge.className = 'status-badge';"
"fixText.innerText = 'Searching (' + data.sats + ' sats)';"
"mapOverlay.style.display = 'flex';"
"}"
"document.getElementById('sats-val').innerText = data.sats || '0';"
"document.getElementById('alt-val').innerText = (data.alt || 0).toFixed(1) + ' m';"
"document.getElementById('speed-val').innerText = (data.speed || 0).toFixed(1) + ' km/h';"
"if (data.time) document.getElementById('gps-time').innerText = data.time;"
"if (data.date) document.getElementById('date-val').innerText = data.date;"
"document.getElementById('qual-val').innerText = data.valid ? 'Active GNSS Tracking' : 'Acquiring satellites...';"
"}"
"function recenterMap() {"
"if (map && currentLat && currentLon) map.setView([currentLat, currentLon], 17);"
"}"
"function copyCoords() {"
"const text = `${currentLat.toFixed(6)}, ${currentLon.toFixed(6)}`;"
"navigator.clipboard.writeText(text).then(() => {"
"const toast = document.getElementById('toast');"
"toast.classList.add('show');"
"setTimeout(() => toast.classList.remove('show'), 2500);"
"});"
"}"
"async function pollGPS() {"
"try {"
"const res = await fetch('/api/gps');"
"if (res.ok) {"
"const data = await res.json();"
"updateUI(data);"
"}"
"} catch (e) {"
"console.warn('GPS polling error', e);"
"}"
"}"
"window.addEventListener('load', () => {"
"initMap();"
"pollGPS();"
"setInterval(pollGPS, 1000);"
"});"
"</script>"
"</body>"
"</html>";

// HTTP Handler for Root "/"
static esp_err_t root_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Connection", "close");
    return httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
}

// HTTP Handler for "/api/gps"
static esp_err_t api_gps_handler(httpd_req_t *req) {
    gps_data_t gps = {0};
    GPS_WebServer_GetData(&gps);

    char json[320];
    snprintf(json, sizeof(json),
        "{\"valid\":%s,\"lat\":%.6f,\"lat_dir\":\"%c\",\"lon\":%.6f,\"lon_dir\":\"%c\","
        "\"alt\":%.2f,\"speed\":%.2f,\"sats\":%d,\"fix\":%d,\"time\":\"%s\",\"date\":\"%s\"}",
        gps.is_valid ? "true" : "false",
        gps.latitude,
        gps.lat_direction ? gps.lat_direction : 'N',
        gps.longitude,
        gps.lon_direction ? gps.lon_direction : 'E',
        gps.altitude,
        gps.speed_kmh,
        gps.satellites,
        gps.fix_quality,
        gps.time[0] ? gps.time : "--:--:--",
        gps.date[0] ? gps.date : "--/--/----"
    );

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    return httpd_resp_send(req, json, strlen(json));
}

esp_err_t GPS_WebServer_Start(void) {
    if (!s_gps_mutex) {
        s_gps_mutex = xSemaphoreCreateMutex();
    }

    ESP_LOGI(TAG, "Initializing Wi-Fi subsystem...");
    esp_err_t err = WIFI_Init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "WIFI_Init failed: %s", esp_err_to_name(err));
        return err;
    }

    // Configure and start Wi-Fi Access Point
    WIFI_AP_InitTypeDef ap_cfg = {
        .SSID = GPS_WIFI_AP_SSID,
        .Password = GPS_WIFI_AP_PASSWORD,
        .Channel = GPS_WIFI_AP_CHANNEL,
        .MaxConnections = GPS_WIFI_AP_MAX_CONN,
    };

#if GPS_WIFI_STA_ENABLE
    WIFI_STA_InitTypeDef sta_cfg = {
        .SSID = GPS_WIFI_STA_SSID,
        .Password = GPS_WIFI_STA_PASSWORD,
        .Channel = 0,
        .MaxRetry = 5,
    };
    ESP_LOGI(TAG, "Starting Wi-Fi Dual AP+STA Mode...");
    err = WIFI_StartSTA_AP(&sta_cfg, &ap_cfg);
#else
    ESP_LOGI(TAG, "Starting Wi-Fi Access Point: SSID='%s', Password='%s'...",
             GPS_WIFI_AP_SSID, GPS_WIFI_AP_PASSWORD);
    err = WIFI_StartAP(&ap_cfg);
#endif

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start Wi-Fi: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Wi-Fi AP active. Connect to '%s' and open http://192.168.4.1", GPS_WIFI_AP_SSID);

    // Start HTTP server
    s_server = HTTP_Start();
    if (!s_server) {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return ESP_FAIL;
    }

    // Register URI handlers
    HTTP_RegisterRoot(s_server, root_handler);
    HTTP_RegisterUri(s_server, "/api/gps", HTTP_GET, api_gps_handler);

    ESP_LOGI(TAG, "GPS Web Server successfully running on port 80!");
    return ESP_OK;
}
