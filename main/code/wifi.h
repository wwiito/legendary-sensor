/*
 * wifi.h
 *
 *  Created on: 12 cze 2020
 *      Author: wit
 */

#ifndef MAIN_CODE_WIFI_H_
#define MAIN_CODE_WIFI_H_

#include "string"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

class wifi {
public:
	wifi() {
		is_connected = false;
		s_wifi_event_group = xEventGroupCreate();
	}
	virtual ~wifi() {}

	void wifi_start_sta();
	void wait_for_ready();
	void wifi_mark_ready(int status_bit);
	bool is_ready() {
		return is_connected;
	}

	EventGroupHandle_t get_wifi_ev_group() {
		return s_wifi_event_group;
	}

	void set_creditentials(std::string name, std::string pass);
	const std::string & get_tag();

	static const int WIFI_CONNNECTED = (1<<0);
	static const int WIFI_FAIL       = (1<<1);

private:
	bool is_connected;

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    EventGroupHandle_t s_wifi_event_group;

	const std::string TAG="wifi";
	std::string wifi_name;
	std::string wifi_pass;
};

#endif /* MAIN_CODE_WIFI_H_ */
