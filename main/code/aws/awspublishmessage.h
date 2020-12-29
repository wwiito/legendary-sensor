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

#include <string>

class aws_mqtt_message {
public:
	aws_mqtt_message(QoS q, void * payload, int payloadLength, const std::string & mqtt_topic)
	:topic(mqtt_topic)
	{
		p.qos = q;
		p.payload = payload;
		p.payloadLen = payloadLength;
		p.isRetained = 0;
	}
	const std::string & get_topic() {return topic;}
	IoT_Publish_Message_Params * get_raw_msg() {return &p;}
private:
	IoT_Publish_Message_Params p;
	const std::string & topic;
};

#endif /* MAIN_CODE_AWSPUBLISHMESSAGE_H_ */
