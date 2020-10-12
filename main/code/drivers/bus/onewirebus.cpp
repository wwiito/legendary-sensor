/*
 * onewirebus.cpp
 *
 *  Created on: Sep 13, 2020
 *      Author: wit
 */

#include "onewirebus.h"

onewire_bus::onewire_bus(gpio_num_t gpio_num, rmt_channel_t tx_channel, rmt_channel_t rx_channel) {
	bool found = false;

	gpio_set_pull_mode(gpio_num, GPIO_PULLUP_ONLY);
    this->owb = owb_rmt_initialize(&(this->rmt_driver_info), gpio_num, tx_channel, rx_channel);
    owb_use_crc(this->owb, true);

    owb_search_first(this->owb, &(this->search_state), &found);

    while (found)
    {
        this->connected_roms.push_back(search_state.rom_code);
        owb_search_next(owb, &search_state, &found);
    }

    for (auto val : connected_roms) {
        char rom_code_s[17];
        bool isp = false;
        owb_string_from_rom_code(val, rom_code_s, sizeof(rom_code_s));
    	owb_verify_rom(owb, val, &isp);
    	ESP_LOGI("OWB","Checking ROM %s", rom_code_s);
    	if (!isp) {
        	ESP_LOGI("OWB","Checking ROM %s: FAIL", rom_code_s);
    	}
    }
}

onewire_bus::~onewire_bus() {
	// TODO Auto-generated destructor stub
}

