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
		throw;
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
	ESP_LOGI("msg", "Copy constructor");
	dtype = m.dtype;
	json_data = m.json_data;
	string_data = m.string_data;
	mqtt_topic = m.mqtt_topic;
	if (m.raw_data.payloadLen != 0) {
		memcpy(&raw_data, &m.raw_data, sizeof(IoT_Publish_Message_Params));
		raw_data.payload = new char[raw_data.payloadLen];
		if (!raw_data.payload)
			throw;
		memcpy(raw_data.payload, m.raw_data.payload, raw_data.payloadLen);
	}
}
aws_mqtt_message::aws_mqtt_message(aws_mqtt_message &&m) {
	ESP_LOGI("msg", "Move constructor");
	dtype = m.dtype;
	json_data = std::move(m.json_data);
	string_data = std::move(m.string_data);
	mqtt_topic = std::move(m.mqtt_topic);
	if (m.raw_data.payloadLen != 0) {
		memcpy(&raw_data, &m.raw_data, sizeof(IoT_Publish_Message_Params));
		m.raw_data.payload = NULL;
		m.raw_data.payloadLen = 0;
	}
}
aws_mqtt_message::~aws_mqtt_message(){
	if (raw_data.payload)
		delete [] ((char*)raw_data.payload);
}

aws_mqtt_message&  aws_mqtt_message::operator=(aws_mqtt_message &&m) {
	ESP_LOGE("msg", "operator=move");
	dtype = m.dtype;
	json_data = std::move(m.json_data);
	string_data = std::move(m.string_data);
	mqtt_topic = std::move(m.mqtt_topic);
	if (m.raw_data.payloadLen != 0) {
		memcpy(&raw_data, &m.raw_data, sizeof(IoT_Publish_Message_Params));
		m.raw_data.payload = NULL;
		m.raw_data.payloadLen = 0;
	}
	return *this;
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
		throw;
	}
	memset(raw_data.payload, 0x00, len);
	memcpy(raw_data.payload, reinterpret_cast<void *>(&from[0]), len);
	raw_data.payloadLen = len;
}

void aws_mqtt_message::json_to_raw(nlohmann::json &j) {
	auto s = j.dump();
	string_to_raw(s);
}

void aws_mqtt_message::raw_to_json() {
	int data_len = raw_data.payloadLen;
	char *data = new char[data_len+1];
	if (!data) {
		throw;
	}
	memset(data, 0x00, data_len+1);
	memcpy(data, raw_data.payload, raw_data.payloadLen);

	std::string s = std::string(data);
	json_data = nlohmann::json::parse(s);
	dtype = aws_mqtt_message::DATA_TYPE_JSON;
	delete [] data;
	if (raw_data.payload) {
		delete [] (char *)raw_data.payload;
		raw_data.payload = NULL;
		raw_data.payloadLen = 0;
	}
}

nlohmann::json & aws_mqtt_message::get_json() {
	return json_data;
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
