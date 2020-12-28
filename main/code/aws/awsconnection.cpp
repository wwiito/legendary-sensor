/*
 * awsconnection.cpp
 *
 *  Created on: Aug 1, 2020
 *      Author: wit
 */

#include "awsconnection.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

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

IoT_Error_t aws_connection_mqtt::publish_msg(aws_publish_message &msg) {
	return aws_iot_mqtt_publish(&(aws_connection::get_client()), msg.get_topic().data(), msg.get_topic().length(), msg.get_raw_msg());
}
