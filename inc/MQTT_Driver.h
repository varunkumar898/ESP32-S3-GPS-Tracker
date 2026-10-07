#ifndef MQTT_DRIVER_H
#define MQTT_DRIVER_H
#include "mqtt_client.h"

typedef struct {
    const char *URI;
    const char *ClientID;
    const char *Username;
    const char *Password;
} MQTT_InitTypeDef;

esp_mqtt_client_handle_t MQTT_Init(const MQTT_InitTypeDef *config);
esp_err_t MQTT_Start(esp_mqtt_client_handle_t client);
esp_err_t MQTT_Stop(esp_mqtt_client_handle_t client);
int MQTT_Publish(esp_mqtt_client_handle_t client, const char *topic,
                 const char *data, int qos, int retain);
int MQTT_Subscribe(esp_mqtt_client_handle_t client, const char *topic, int qos);

#endif
