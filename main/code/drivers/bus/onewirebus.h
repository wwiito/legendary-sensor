/*
 * onewirebus.h
 *
 *  Created on: Sep 13, 2020
 *      Author: wit
 */

#ifndef MAIN_CODE_DRIVERS_ONEWIREBUS_H_
#define MAIN_CODE_DRIVERS_ONEWIREBUS_H_

#include <vector>
#include <memory>

#include "owb.h"
#include "owb_rmt.h"

#include "esp_log.h"


class onewire_bus {
public:
	onewire_bus(gpio_num_t gpio_num, rmt_channel_t tx_channel, rmt_channel_t rx_channel);
	virtual ~onewire_bus();

	std::vector<OneWireBus_ROMCode> get_attached_ROM() {
		return connected_roms;
	}

	void write_byte(uint8_t byte) {
		if (owb_write_byte(owb, byte) != OWB_STATUS_OK)
			throw std::runtime_error("ONEWIRE: Failed to write byte");
	}

	void reset_devices() {
		bool t = true;
		owb_reset(owb, &t);
	}

	std::vector<uint8_t> read_bytes(int count) {
		std::vector<uint8_t> v;
		uint8_t *  buf = new uint8_t[count];
		if (buf == NULL) {
			throw std::runtime_error("ONEWIRE: Failed to allocate buffer");
		}
		if (owb_read_bytes(owb, buf, count) != OWB_STATUS_OK)
			throw std::runtime_error("ONEWIRE: Failed to read bytes");

		for (int i = 0; i < count ; i++) {
			v.push_back(buf[i]);
		}

		delete [] buf;
		return v;
	}

	void write_rom(OneWireBus_ROMCode r) {
		if (owb_write_rom_code(owb, r) != OWB_STATUS_OK)
			throw std::runtime_error("ONEWIRE: Failed to write ROM code");
	}
private:
    OneWireBus * owb;
    owb_rmt_driver_info rmt_driver_info;

    std::vector<OneWireBus_ROMCode> connected_roms;
    OneWireBus_SearchState search_state = {};
};

#endif /* MAIN_CODE_DRIVERS_ONEWIREBUS_H_ */
