/*
 * mqttclient.h
 *
 *  Created on: Jun 30, 2020
 *      Author: wit
 */

#ifndef MAIN_CODE_MQTTCLIENT_H_
#define MAIN_CODE_MQTTCLIENT_H_

#include "mqtt_client.h"
#include "esp_log.h"

class mqtt_client {
public:
	mqtt_client() {
		connected = false;
		subscribed = false;
	}
	virtual ~mqtt_client() {}

	void start_client();
	void on_connect(bool connect_status);
	void on_subscribed();

	void test() {
		ESP_LOGI("sasa", "aa");
		esp_mqtt_client_publish(client, "domoticz/in", "{ \"idx\" : 1, \"nvalue\" : 0, \"svalue\" : \"25.0\" }", 0, 0, 0);
	}

	bool is_ready() {
		return connected && subscribed;
	}
private:
	esp_mqtt_client_handle_t client;
	bool connected;
	bool subscribed;
};

#endif /* MAIN_CODE_MQTTCLIENT_H_ */
