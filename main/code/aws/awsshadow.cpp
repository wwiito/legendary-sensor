/*
 * awsshadow.cpp
 *
 *  Created on: Jan 1, 2021
 *      Author: wit
 */

#include "awsshadow.h"

awsshadow::awsshadow(aws_connection_mqtt &c, std::string &tname)
:connection(c),
 thing_name(tname)
{
	lock_sem.give();
	c.register_message_handler(generate_topic(TOPIC_GET_ACCEPTED), receive_handler, this);
	c.attach_topic(generate_topic(TOPIC_GET_ACCEPTED));
	aws_mqtt_message m = aws_mqtt_message(std::string("{}"), generate_topic(TOPIC_GET));
	c.publish_msg(m);
}

awsshadow::~awsshadow() {
	// TODO Auto-generated destructor stub
}

void awsshadow::receive_handler(aws_mqtt_message m, void *ptr) {
	awsshadow *s = (awsshadow *)ptr;
	s->on_receive(std::move(m));
}

void awsshadow::on_receive(aws_mqtt_message m) {
	this->m = std::move(m);
	//Prepare new topic so shadow can be updated
	this->m.raw_to_json();
	this->m.change_topic(generate_topic(awsshadow::TOPIC_UPDATE));
	data_ready.give();
}

std::string awsshadow::generate_base_topic() {
	std::string tmp = "$aws/things/" + this->thing_name;
	return tmp;
}

std::string awsshadow::generate_topic(enum awsshadow::topic_type type) {
	std::string s;
	switch(type) {
	case awsshadow::TOPIC_UPDATE:
		s = this->generate_base_topic() + "/shadow/update";
		break;
	case awsshadow::TOPIC_GET:
		s = this->generate_base_topic() + "/shadow/get";
		break;
	case awsshadow::TOPIC_GET_ACCEPTED:
		s = this->generate_base_topic() + "/shadow/get/accepted";
		break;
	}
	return s;
}
