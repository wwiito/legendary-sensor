/*
 * aws.cpp
 *
 *  Created on: Jul 11, 2020
 *      Author: wit
 */

#include "aws.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

aws::aws() {
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

IoT_Error_t aws::init_mqtt() {
	IoT_Error_t rc = aws_iot_mqtt_init(&client, &mqtt_initParams);
	if (SUCCESS != rc) {
		ESP_LOGE(TAG.data(), "Failed to init aws mqtt!");
	} else {
		ESP_LOGI(TAG.data(), "AWS mqtt initialized!");
	}
	return rc;
}

IoT_Error_t aws::connect_mqtt(int reconnect_count) {
	IoT_Error_t rc;

	ESP_LOGI(TAG.data(), "Connecting to AWS");
	do {
		if (reconnect_count > 0)
			reconnect_count --;

		rc = aws_iot_mqtt_connect(&client, &mqtt_connectParams);
		if (SUCCESS != rc) {
			ESP_LOGE(TAG.data(), "Error(%d) connecting to %s:%d retries left: %d", rc, mqtt_initParams.pHostURL, mqtt_initParams.port, reconnect_count);
			vTaskDelay(1000/portTICK_RATE_MS);
		} else {
			ESP_LOGI(TAG.data(), "Connected to AWS MQTT");
		}
	} while((SUCCESS != rc) && (reconnect_count != 0));

	return rc;
}

IoT_Error_t aws::publish_msg(aws_mqtt_message &msg) {
	return aws_iot_mqtt_publish(&client, msg.get_topic().data(), msg.get_topic().length(), msg.get_raw_msg());
}





