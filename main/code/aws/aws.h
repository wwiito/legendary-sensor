/*
 * aws.h
 *
 *  Created on: Jul 11, 2020
 *      Author: wit
 */

#ifndef MAIN_CODE_AWS_H_
#define MAIN_CODE_AWS_H_

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"

#include <string>

#include "espfile.h"

#include "awspublishmessage.h"
#include "awsconnection.h"

class aws {
public:
	aws();
	virtual ~aws() {};

	IoT_Error_t init_mqtt();
	IoT_Error_t connect_mqtt(int reconnect_count=-1);

	IoT_Error_t init_shadow();
	IoT_Error_t connect_shadow();

	IoT_Error_t yeld(int yeld_timeout=10) {return aws_iot_mqtt_yield(&client, yeld_timeout);}
	IoT_Error_t publish_msg(aws_publish_message &msg);

	void autoReconnectStatus(bool status) { mqtt_initParams.enableAutoReconnect = status;}
	void endpoint_URL(std::string &URL) {mqtt_initParams.pHostURL = reinterpret_cast<char *>(&URL[0]);}
	void endpoint_port(int port) {mqtt_initParams.port = static_cast<uint16_t>(port);}
	void root_CA(esp_file &f) {mqtt_initParams.pRootCALocation = reinterpret_cast<const char *>(f.get_file_buffer());}
	void device_Cert(esp_file &f) {mqtt_initParams.pDeviceCertLocation = reinterpret_cast<const char *>(f.get_file_buffer());}
	void private_Key(esp_file &f) {mqtt_initParams.pDevicePrivateKeyLocation = reinterpret_cast<const char *>(f.get_file_buffer());}
	void client_ID(std::string &id);
	void device_name(std::string &name);
private:
	AWS_IoT_Client client;
	IoT_Client_Init_Params    mqtt_initParams;
	IoT_Client_Connect_Params mqtt_connectParams;
	ShadowInitParameters_t    shadow_initParams;
	ShadowConnectParameters_t shadow_connectParams;

	std::string client_id;
	std::string thing_name;
	std::string TAG = "AWS";
};

#endif /* MAIN_CODE_AWS_H_ */
