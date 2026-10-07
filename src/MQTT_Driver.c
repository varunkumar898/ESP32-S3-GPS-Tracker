#include "MQTT_Driver.h"

static void mqtt_event_handler(void *args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    (void)args; (void)base; (void)event_id; (void)event_data;
}

esp_mqtt_client_handle_t MQTT_Init(const MQTT_InitTypeDef *c)
{
    if (!c || !c->URI) return NULL;
    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = c->URI,
        .credentials.client_id = c->ClientID,
        .credentials.username = c->Username,
        .credentials.authentication.password = c->Password,
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&cfg);
    if (!client) return NULL;
    if (esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID,
                                       mqtt_event_handler, NULL) != ESP_OK) {
        esp_mqtt_client_destroy(client);
        return NULL;
    }
    return client;
}

esp_err_t MQTT_Start(esp_mqtt_client_handle_t client)
{
    return client ? esp_mqtt_client_start(client) : ESP_ERR_INVALID_ARG;
}

esp_err_t MQTT_Stop(esp_mqtt_client_handle_t client)
{
    return client ? esp_mqtt_client_stop(client) : ESP_ERR_INVALID_ARG;
}

int MQTT_Publish(esp_mqtt_client_handle_t client, const char *topic,
                 const char *data, int qos, int retain)
{
    if (!client || !topic || !data) return -1;
    return esp_mqtt_client_publish(client, topic, data, 0, qos, retain);
}

int MQTT_Subscribe(esp_mqtt_client_handle_t client, const char *topic, int qos)
{
    if (!client || !topic) return -1;
    return esp_mqtt_client_subscribe(client, topic, qos);
}
