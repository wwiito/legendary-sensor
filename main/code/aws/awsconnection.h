
#ifndef MAIN_CODE_AWS_AWSCONNECTION_H_
#define MAIN_CODE_AWS_AWSCONNECTION_H_

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"
#include <string>

#include "awsmessage.h"
#include "espfile.h"

#include <functional>
#include <map>
#include <tuple>


class aws_connection {
public:
	aws_connection(){};
	virtual ~aws_connection(){};

	virtual IoT_Error_t init_connection() = 0;
	virtual IoT_Error_t connect(int reconnect_count = 3) = 0;
	AWS_IoT_Client & get_client(){return client;}
	IoT_Error_t yeld(int yeld_timeout=10) {return aws_iot_mqtt_yield(&client, yeld_timeout);}

	virtual void autoReconnectStatus(bool status) = 0;
	virtual void endpoint_URL(std::string &URL) = 0;
	virtual void endpoint_port(int port) = 0;
	virtual void root_CA(esp_file &f) = 0;
	virtual void device_Cert(esp_file &f) = 0;
	virtual void private_Key(esp_file &f) = 0;
	virtual void client_ID(std::string &id) = 0;
	virtual void device_name(std::string &name) = 0;

private:
	AWS_IoT_Client client;
};

class aws_connection_mqtt : public aws_connection {
public:
	aws_connection_mqtt();
	~aws_connection_mqtt() {};
	IoT_Error_t init_connection();
	IoT_Error_t connect(int reconnect_count);
	IoT_Error_t update_shadow_status();
	IoT_Error_t publish_shadow_update();

	void autoReconnectStatus(bool status) { mqtt_initParams.enableAutoReconnect = status;}
	void endpoint_URL(std::string &URL) {mqtt_initParams.pHostURL = reinterpret_cast<char *>(&URL[0]);}
	void endpoint_port(int port) {mqtt_initParams.port = static_cast<uint16_t>(port);}
	void root_CA(esp_file &f) {mqtt_initParams.pRootCALocation = reinterpret_cast<const char *>(f.get_file_buffer());}
	void device_Cert(esp_file &f) {mqtt_initParams.pDeviceCertLocation = reinterpret_cast<const char *>(f.get_file_buffer());}
	void private_Key(esp_file &f) {mqtt_initParams.pDevicePrivateKeyLocation = reinterpret_cast<const char *>(f.get_file_buffer());}
	void client_ID(std::string &id);
	void device_name(std::string &name) {thing_name = name;}
	IoT_Error_t publish_msg(aws_mqtt_message &msg);
	IoT_Error_t attach_topic(std::string topic);
	void on_receive(aws_mqtt_message m);
	static void receive_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, IoT_Publish_Message_Params *params, void *pData);

	void register_message_handler(std::string topic, void (*)(aws_mqtt_message, void*), void *param);

	std::string construct_update_topic() {
		std::string tmp = construct_base_topic() + "/shadow/update";
		return tmp;
	}
	std::string construct_get_topic() {
		std::string tmp = construct_base_topic() + "/shadow/get/accepted";
		return tmp;
	}
	std::string construct_fetch_topic() {
		std::string tmp = construct_base_topic() + "/shadow/get";
		return tmp;
	}

private:
	IoT_Client_Init_Params    mqtt_initParams;
	IoT_Client_Connect_Params mqtt_connectParams;

	std::string construct_base_topic() {
		std::string tmp = "$aws/things/" + this->thing_name;
		return tmp;
	}

	std::string client_id;
	std::string thing_name;
	std::map<std::string, std::tuple<void (*)(aws_mqtt_message, void*), void *>> msg_client_map;

	std::string TAG = "AWS_CONNECTION_MQTT";
	static bool param_log_received_messages;
};

#endif /* MAIN_CODE_AWS_AWSCONNECTION_H_ */
