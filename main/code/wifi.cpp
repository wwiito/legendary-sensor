/*
 * wifi.cpp
 *
 *  Created on: 12 cze 2020
 *      Author: wit
 */

#include "wifi.h"
#include "string.h"

const std::string EV_LOOP_TAG="Wifi_ev_loop";
int aa = 10;

void ev_h(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {

	wifi *w = reinterpret_cast<wifi*>(arg);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(EV_LOOP_TAG.data(), "event start");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(EV_LOOP_TAG.data(), "retry to connect to the AP");
    }
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI("Wifi_ev_loop", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        w->wifi_mark_ready(wifi::WIFI_CONNNECTED);
    }
}

void wifi::wait_for_ready() {
    xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNNECTED,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);
    ESP_LOGI(TAG.data(), "wait for ip finished");
}

void wifi::set_creditentials(std::string name, std::string pass) {
	wifi_name = name;
	wifi_pass = pass;
}

const std::string & wifi::get_tag() {
	return this->TAG;
}

void wifi::wifi_mark_ready(int status_bit) {
	xEventGroupSetBits(s_wifi_event_group, status_bit);
}

void wifi::wifi_start_sta() {
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
    									&ev_h,
										reinterpret_cast<void*>(this),
                                        &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT,IP_EVENT_STA_GOT_IP,
    									&ev_h,
										reinterpret_cast<void*>(this),
                                        &instance_got_ip);

    wifi_config_t wifi_config = {};
    memcpy(wifi_config.sta.ssid, wifi_name.data(), wifi_name.length()+1);
    memcpy(wifi_config.sta.password, wifi_pass.data(), wifi_pass.length()+1);
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG.data(), "wifi_init_sta finished.");
}
