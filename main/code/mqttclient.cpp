/*
 * mqttclient.cpp
 *
 *  Created on: Jun 30, 2020
 *      Author: wit
 */

#include "mqttclient.h"

#define TAG "aaa"

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    mqtt_client *c = reinterpret_cast<mqtt_client *>(handler_args);

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            c->on_connect(true);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            c->on_subscribed();
            ESP_LOGI(TAG, "sent publish successful, msg_id");
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            //printf("TOPIC=%\n.*s\r\n", event->topic_len, event->topic);
            //printf("DATA=%.*s\r", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

void mqtt_client::on_connect(bool connect_status) {
	if (connect_status) {
		esp_mqtt_client_subscribe(client, "/domoticz/in", 1);
		connected = true;
	} else {
		connected = false;
		subscribed = false;
	}
}

void mqtt_client::on_subscribed() {
	subscribed = true;
}

void mqtt_client::start_client() {
    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.uri = "mqtt://192.168.88.19";

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, reinterpret_cast<void*>(this));
    esp_mqtt_client_start(client);
}

