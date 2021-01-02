/*
 * awspublishmessage.cpp
 *
 *  Created on: Aug 1, 2020
 *      Author: wit
 */

#include <awsmessage.h>

aws_mqtt_message::aws_mqtt_message(void * payload, int payloadLength, std::string topic, QoS q)
{
	base_raw_init(q);
	raw_data.payload = new char[payloadLength];
	raw_data.payloadLen = payloadLength;
	if (!raw_data.payload) {
		dtype = aws_mqtt_message::DATA_FAIL;
		return;
	} else {
		memcpy(raw_data.payload, payload, payloadLength);
	}
	mqtt_topic = topic;
	dtype = aws_mqtt_message::DATA_TYPE_RAW;
}

aws_mqtt_message::aws_mqtt_message(std::string payload, std::string topic, QoS q)
{
	base_raw_init(q);
	mqtt_topic = topic;
	string_data = payload;
	dtype = aws_mqtt_message::DATA_TYPE_STRING;
}
aws_mqtt_message::aws_mqtt_message(nlohmann::json j, std::string topic, QoS q)
{
	base_raw_init(q);
	mqtt_topic = topic;
	json_data = j;
	dtype = aws_mqtt_message::DATA_TYPE_JSON;
}
aws_mqtt_message::aws_mqtt_message(const aws_mqtt_message &m) {

}
aws_mqtt_message::~aws_mqtt_message(){
	if (raw_data.payload)
		delete [] ((char*)raw_data.payload);
}


void aws_mqtt_message::base_raw_init(QoS q) {
	raw_data.qos = q;
	raw_data.isRetained = 0;
}
void aws_mqtt_message::string_to_raw(std::string &from) {
	int len = from.length();
	raw_data.payload = new char[len];
	if (!raw_data.payload) {
		ESP_LOGE("msg", "Failed to allocate memory");
		return;
	}
	memset(raw_data.payload, 0x00, len);
	memcpy(raw_data.payload, reinterpret_cast<void *>(&from[0]), len);
	raw_data.payloadLen = len;
}

void aws_mqtt_message::json_to_raw(nlohmann::json &j) {
	auto s = j.dump();
	string_to_raw(s);
}

IoT_Publish_Message_Params * aws_mqtt_message::get_raw_msg() {
	switch (dtype){
	case aws_mqtt_message::DATA_TYPE_RAW:
		break;
	case aws_mqtt_message::DATA_TYPE_STRING:
		string_to_raw(string_data);
		break;
	case aws_mqtt_message::DATA_TYPE_JSON:
		json_to_raw(json_data);
		break;
	default:
		break;
	}
	return &raw_data;
}
