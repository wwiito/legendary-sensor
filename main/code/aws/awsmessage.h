/*
 * awspublishmessage.h
 *
 *  Created on: Aug 1, 2020
 *      Author: wit
 */

#ifndef MAIN_CODE_AWSPUBLISHMESSAGE_H_
#define MAIN_CODE_AWSPUBLISHMESSAGE_H_

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"

#include "json.hpp"

#include <string>

class aws_mqtt_message {
public:
	aws_mqtt_message() {};
	aws_mqtt_message(void * payload, int payloadLength, std::string topic, QoS q = QOS0);
	aws_mqtt_message(const std::string payload, std::string topic, QoS q = QOS0);
	aws_mqtt_message(nlohmann::json j, std::string topic, QoS q = QOS0);
	aws_mqtt_message(const aws_mqtt_message &m);
	aws_mqtt_message(aws_mqtt_message &&m);
	virtual ~aws_mqtt_message();
	aws_mqtt_message&  operator=(aws_mqtt_message &&m);

	const std::string & get_topic() {return mqtt_topic;}
	void change_topic(std::string s) {mqtt_topic = s;}
	IoT_Publish_Message_Params * get_raw_msg();
	void raw_to_json();
	nlohmann::json & get_json();
	void print_type() {
		ESP_LOGE("hmpf", "%d", dtype);
	}
private:
	void base_raw_init(QoS q);
	void string_to_raw(std::string &from);
	void json_to_raw(nlohmann::json &j);
	enum data_type {
		DATA_TYPE_RAW=0,
		DATA_TYPE_STRING,
		DATA_TYPE_JSON,
		DATA_FAIL
	};
	enum data_type dtype;
	IoT_Publish_Message_Params raw_data;
	nlohmann::json json_data;
	std::string string_data;
	std::string mqtt_topic;
};

#endif /* MAIN_CODE_AWSPUBLISHMESSAGE_H_ */
