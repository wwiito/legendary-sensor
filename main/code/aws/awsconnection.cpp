/*
 * awsconnection.cpp
 *
 *  Created on: Aug 1, 2020
 *      Author: wit
 */

#include "awsconnection.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <cstring>

std::string aws_connection_mqtt::TAG = "AWS_CONNECTION_MQTT";
bool aws_connection_mqtt::param_log_received_messages = true;

void aws_connection_mqtt::receive_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, IoT_Publish_Message_Params *params, void *pData) {
	aws_connection_mqtt *t = (aws_connection_mqtt*)pData;
	ESP_LOGI(TAG.data(), "In receive handler");
	if (aws_connection_mqtt::param_log_received_messages) {
		char *tmp_topic = new char [topicNameLen+1];
		char *tmp_payload = new char[params->payloadLen+1];
		if(!tmp_topic || !tmp_payload) {
			ESP_LOGI(TAG.data(), "Memory allocation fail");
			return;
		}
		memset(tmp_topic, 0x00, topicNameLen+1);
		memset(tmp_payload, 0x00, params->payloadLen+1);

		memcpy(tmp_topic, topicName, topicNameLen);
		memcpy(tmp_payload, params->payload, params->payloadLen);

		ESP_LOGI(TAG.data(), "topic name: %s", tmp_topic);
		ESP_LOGI(TAG.data(), "Payload: %s", tmp_payload);
		delete[](tmp_topic);
		delete[](tmp_payload);
	}
	t->on_receive();
}

aws_connection_mqtt::aws_connection_mqtt() {
	mqtt_initParams = iotClientInitParamsDefault;
	mqtt_connectParams = iotClientConnectParamsDefault;

	/* Basic init and connect parameters */
	mqtt_initParams.enableAutoReconnect = false;
	mqtt_initParams.port = 8883;
	mqtt_connectParams.keepAliveIntervalInSec = 10;
	mqtt_connectParams.isCleanSession = true;
	mqtt_connectParams.MQTTVersion = MQTT_3_1_1;
	mqtt_connectParams.isWillMsgPresent = false;
}

void aws_connection_mqtt::client_ID(std::string &id) {
	mqtt_connectParams.pClientID = reinterpret_cast<const char *>(id.data());
	mqtt_connectParams.clientIDLen = id.length();
	client_id = std::string(id);
}

IoT_Error_t aws_connection_mqtt::init_connection() {
	IoT_Error_t rc = aws_iot_mqtt_init(&(aws_connection::get_client()), &mqtt_initParams);
	if (SUCCESS != rc) {
		ESP_LOGE(TAG.data(), "Failed to init aws mqtt!");
	} else {
		ESP_LOGI(TAG.data(), "AWS mqtt initialized!");
	}
	return rc;
}

IoT_Error_t aws_connection_mqtt::update_shadow_status() {
	return SUCCESS;
}

IoT_Error_t aws_connection_mqtt::publish_shadow_update() {
	return SUCCESS;
}

IoT_Error_t aws_connection_mqtt::connect(int reconnect_count = 3) {
	IoT_Error_t rc;

	ESP_LOGI(TAG.data(), "Connecting to AWS");
	do {
		if (reconnect_count > 0)
			reconnect_count --;

		rc = aws_iot_mqtt_connect(&(aws_connection::get_client()), &mqtt_connectParams);
		if (SUCCESS != rc) {
			ESP_LOGE(TAG.data(), "Error(%d) connecting to %s:%d retries left: %d", rc, mqtt_initParams.pHostURL, mqtt_initParams.port, reconnect_count);
			vTaskDelay(1000/portTICK_RATE_MS);
		} else {
			ESP_LOGI(TAG.data(), "Connected to AWS MQTT");
		}
	} while((SUCCESS != rc) && (reconnect_count != 0));

	return rc;
}

IoT_Error_t aws_connection_mqtt::publish_msg(aws_mqtt_message &msg) {
	return aws_iot_mqtt_publish(&(aws_connection::get_client()), msg.get_topic().data(), msg.get_topic().length(), msg.get_raw_msg());
}

IoT_Error_t aws_connection_mqtt::attach_topic(std::string &topic) {
	char *tmp_topic = new char [topic.length()];
	if (!tmp_topic)
		return FAILURE;

	memcpy(tmp_topic, topic.data(), topic.length());
	return aws_iot_mqtt_subscribe(&(aws_connection::get_client()), tmp_topic, topic.length(), QOS0, aws_connection_mqtt::receive_handler, this);
}

void aws_connection_mqtt::on_receive() {
	ESP_LOGI(TAG.data(), "on_receive");
}
