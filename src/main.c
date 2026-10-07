#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "ESP32S3_Driver.h"
#include "GPS_Driver.h"
#include "GPS_WebServer.h"

static const char *TAG = "APP_MAIN";

// FreeRTOS Task to read GPS NMEA stream, update Web Server & display in terminal
static void gps_monitor_task(void *pvParameters) {
    char sentence_buf[256];
    gps_data_t gps_data = {0};
    TickType_t last_print_tick = 0;

    ESP_LOGI(TAG, "GPS Monitor Task active on UART Port %d (RX: GPIO %d, TX: GPIO %d)",
             GPS_UART_PORT, GPS_DEFAULT_RX_PIN, GPS_DEFAULT_TX_PIN);

    while (1) {
        // Read raw NMEA sentence line from GPS UART (Pin 17 RX)
        if (GPS_ReadSentence(GPS_UART_PORT, sentence_buf, sizeof(sentence_buf), 1000)) {
            // Parse sentence ($GPRMC, $GNRMC, $GPGGA, $GNGGA)
            if (GPS_ParseNMEA(sentence_buf, &gps_data)) {
                // Update live telemetry for Web Dashboard
                GPS_WebServer_UpdateData(&gps_data);

                // Print location to serial terminal every 1.5 seconds
                TickType_t now = xTaskGetTickCount();
                if ((now - last_print_tick) >= pdMS_TO_TICKS(1500)) {
                    GPS_PrintLocation(&gps_data);
                    last_print_tick = now;
                }
            }
        } else {
            // If satellite fix is searching or idle
            TickType_t now = xTaskGetTickCount();
            if ((now - last_print_tick) >= pdMS_TO_TICKS(3000)) {
                if (!gps_data.is_valid) {
                    GPS_PrintLocation(&gps_data);
                }
                last_print_tick = now;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void) {
    // Initialize NVS Flash
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP32S3_PrintChipInfo();

    printf("\r\n========================================================\r\n");
    printf("   ESP32-S3 GPS Wi-Fi Web Tracker & Real-Time Monitor   \r\n");
    printf("========================================================\r\n");
    printf(" Hardware Connections:\r\n");
    printf("  - ESP32 RX  (GPIO %d)  <--->  GPS TX Pin\r\n", GPS_DEFAULT_RX_PIN);
    printf("  - ESP32 TX  (GPIO %d)  <--->  GPS RX Pin\r\n", GPS_DEFAULT_TX_PIN);
    printf("  - ESP32 GND            <--->  GPS GND\r\n");
    printf("  - ESP32 3.3V / 5V      <--->  GPS VCC\r\n");
    printf("--------------------------------------------------------\r\n");
    printf(" Wi-Fi Web Dashboard Access:\r\n");
#if GPS_WIFI_STA_ENABLE
    printf("  [Station Mode] Connecting to : '%s'\r\n", GPS_WIFI_STA_SSID);
    printf("                 (IP will be printed on connection)\r\n");
#endif
    printf("  [Hotspot Mode] SSID          : '%s'\r\n", GPS_WIFI_AP_SSID);
    printf("                 Password      : '%s'\r\n", GPS_WIFI_AP_PASSWORD);
    printf("                 Default URL   : http://192.168.4.1\r\n");
    printf("========================================================\r\n\n");

    // 1. Initialize GPS UART Driver
    gps_config_t gps_cfg = {
        .uart_port = GPS_UART_PORT,
        .rx_pin = GPS_DEFAULT_RX_PIN,
        .tx_pin = GPS_DEFAULT_TX_PIN,
        .baud_rate = GPS_DEFAULT_BAUD
    };
    ESP_ERROR_CHECK(GPS_Init(&gps_cfg));

    // 2. Start Wi-Fi Access Point and Web Server
    ESP_ERROR_CHECK(GPS_WebServer_Start());

    // 3. Create FreeRTOS task to process GPS stream and update web server
    xTaskCreatePinnedToCore(gps_monitor_task, "gps_task", 4096, NULL, 5, NULL, 1);
}