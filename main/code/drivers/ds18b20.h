/*
 * ds18b20.h
 *
 *  Created on: Sep 10, 2020
 *      Author: wit
 */

#ifndef MAIN_CODE_DRIVERS_DS18B20_H_
#define MAIN_CODE_DRIVERS_DS18B20_H_

#include <vector>
#include <map>
#include "onewirebus.h"

#include "esp_log.h"


union ds18b20_scratchpad {
	uint8_t buffer[9];
	struct scratch {
    	uint8_t temperature[2];    // [0] is LSB, [1] is MSB
    	uint8_t trigger_high;
    	uint8_t trigger_low;
    	uint8_t configuration;
    	uint8_t reserved[3];
    	uint8_t crc;
	}__attribute__((packed)) regs;
};

class ds18b20 {
public:
	ds18b20(onewire_bus &b, OneWireBus_ROMCode &r);
	virtual ~ds18b20();
	void update_scratchpad();
	void print_scratchpad();
	void convert_temperature();
	void all_dev_convert_temperature();
	double get_temperature(bool refresh = false);

private:
	OneWireBus_ROMCode serial_rom;
	onewire_bus &bus;
	bool single_device;
	ds18b20_scratchpad scratchpad;

	void adress_dev();
};

class ds18b20_factory {
public:
	static std::vector<ds18b20> get_all_ds18b20_vec(onewire_bus &bus) {
		std::vector<OneWireBus_ROMCode> roms = bus.get_attached_ROM();
		std::vector<ds18b20> devices;

		for (auto &rom : roms) {
			if (rom.fields.family[0] == 0x28) {
				ds18b20 d = ds18b20(bus, rom);
				devices.push_back(d);
			}
		}
		return devices;
	}
	static std::map<std::string, ds18b20*> get_all_ds18b20_map(onewire_bus &bus) {
		char *rom_char = new char [17];
		std::vector<OneWireBus_ROMCode> roms = bus.get_attached_ROM();
		std::map<std::string, ds18b20*> devices;

		for (auto &rom : roms) {
			if (rom.fields.family[0] == 0x28) {
				owb_string_from_rom_code(rom, rom_char, sizeof(rom_char));
				std::string rom_string(rom_char);
				ds18b20 *d = new ds18b20(bus, rom);
				devices[rom_string] = d;
			}
		}
		delete [] rom_char;
		return devices;
	}
};



#endif /* MAIN_CODE_DRIVERS_DS18B20_H_ */
