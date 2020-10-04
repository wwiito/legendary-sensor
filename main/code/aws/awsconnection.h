
#ifndef MAIN_CODE_AWS_AWSCONNECTION_H_
#define MAIN_CODE_AWS_AWSCONNECTION_H_

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"
#include "awspublishmessage.h"

#include <string>

#include "espfile.h"


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

	void autoReconnectStatus(bool status) { mqtt_initParams.enableAutoReconnect = status;}
	void endpoint_URL(std::string &URL) {mqtt_initParams.pHostURL = reinterpret_cast<char *>(&URL[0]);}
	void endpoint_port(int port) {mqtt_initParams.port = static_cast<uint16_t>(port);}
	void root_CA(esp_file &f) {mqtt_initParams.pRootCALocation = reinterpret_cast<const char *>(f.get_file_buffer());}
	void device_Cert(esp_file &f) {mqtt_initParams.pDeviceCertLocation = reinterpret_cast<const char *>(f.get_file_buffer());}
	void private_Key(esp_file &f) {mqtt_initParams.pDevicePrivateKeyLocation = reinterpret_cast<const char *>(f.get_file_buffer());}
	void client_ID(std::string &id);
	void device_name(std::string &name) {};
	IoT_Error_t publish_msg(aws_publish_message &msg);

private:
	IoT_Client_Init_Params    mqtt_initParams;
	IoT_Client_Connect_Params mqtt_connectParams;

	std::string client_id;
	std::string TAG = "AWS_CONNECTION_MQTT";
};

#endif /* MAIN_CODE_AWS_AWSCONNECTION_H_ */