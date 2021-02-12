/*
 * awsshadow.h
 *
 *  Created on: Jan 1, 2021
 *      Author: wit
 */

#ifndef MAIN_CODE_AWS_AWSSHADOW_H_
#define MAIN_CODE_AWS_AWSSHADOW_H_

#include "awsconnection.h"
#include "awsmessage.h"
#include "rtos_semaphore.h"
#include "json.hpp"

class awsshadow {
public:
	awsshadow(aws_connection_mqtt &c, std::string &tname);
	virtual ~awsshadow();

	enum topic_type{
		TOPIC_UPDATE=0,
		TOPIC_GET,
		TOPIC_GET_ACCEPTED
	};
	std::string generate_topic(enum awsshadow::topic_type type);
	void publish() {
		connection.publish_msg(m);
		//Make sure that msg was published
		for (int i=0; i<2; i++) {
			connection.yeld(1);
			vTaskDelay(5);
		}
	}
	static void receive_handler(aws_mqtt_message m, void *ptr);
	void on_receive(aws_mqtt_message m);
	void wait_shadow_ready() {
		do {
			connection.yeld(1);
			vTaskDelay(5);
		} while (!data_ready.is_ready());
	}
	void lock_shadow() {
		lock_sem.take();
	}
	void unlock_shadow() {
		lock_sem.give();
	}
	nlohmann::json& get_shadow_json() {
		return m.get_json();
	}
private:
	std::string generate_base_topic();
	aws_connection_mqtt &connection;
	std::string &thing_name;
	aws_mqtt_message m;
	rtos_semaphore lock_sem;
	rtos_semaphore data_ready;
};

#endif /* MAIN_CODE_AWS_AWSSHADOW_H_ */
