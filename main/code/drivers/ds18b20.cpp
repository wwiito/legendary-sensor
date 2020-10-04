/*
 * ds18b20.cpp
 *
 *  Created on: Sep 10, 2020
 *      Author: wit
 */

#include "ds18b20.h"
#include <cstring>

#include <freertos/task.h>


#define DS18B20_FUNCTION_TEMP_CONVERT       0x44  ///< Initiate a single temperature conversion
#define DS18B20_FUNCTION_SCRATCHPAD_WRITE   0x4E  ///< Write 3 bytes of data to the device scratchpad at positions 2, 3 and 4
#define DS18B20_FUNCTION_SCRATCHPAD_READ    0xBE  ///< Read 9 bytes of data (including CRC) from the device scratchpad
#define DS18B20_FUNCTION_SCRATCHPAD_COPY    0x48  ///< Copy the contents of the scratchpad to the device EEPROM
#define DS18B20_FUNCTION_EEPROM_RECALL      0xB8  ///< Restore alarm trigger values and configuration data from EEPROM to the scratchpad
#define DS18B20_FUNCTION_POWER_SUPPLY_READ  0xB4  ///< Determine if a device is using parasitic power

ds18b20::ds18b20(onewire_bus &b, OneWireBus_ROMCode &r):
bus(b)
{
	memcpy(this->serial_rom.bytes, r.bytes, sizeof(this->serial_rom.bytes));
	update_scratchpad();
}

ds18b20::~ds18b20() {
	// TODO Auto-generated destructor stub
}

void ds18b20::adress_dev() {
	if (this->single_device) {
		bus.write_byte(OWB_ROM_SKIP);
	} else {
		bus.write_byte(OWB_ROM_MATCH);
		bus.write_rom(this->serial_rom);
	}
}

void ds18b20::update_scratchpad() {
	try {
		bus.reset_devices();
		adress_dev();
		bus.write_byte(DS18B20_FUNCTION_SCRATCHPAD_READ);
		auto v = bus.read_bytes(sizeof(ds18b20_scratchpad));
		for (int i=0; i<v.size(); i++) {
			scratchpad.buffer[i] = v[i];
		}
	} catch (...) {
		ESP_LOGI("DS18B20", "Faile during scratchpad read");
		return;
	}
}

void ds18b20::convert_temperature() {
	try {
		bus.reset_devices();
		adress_dev();
		bus.write_byte(DS18B20_FUNCTION_TEMP_CONVERT);
		vTaskDelay(1000/portTICK_PERIOD_MS);
		update_scratchpad();
	} catch (...) {
		ESP_LOGI("DS18B20", "Faile during scratchpad read");
		return;
	}
}

void ds18b20::all_dev_convert_temperature() {
	try {
		bus.reset_devices();
		bus.write_byte(OWB_ROM_SKIP);
		bus.write_byte(DS18B20_FUNCTION_TEMP_CONVERT);
		vTaskDelay(1000/portTICK_PERIOD_MS);
	} catch (...) {
		ESP_LOGI("DS18B20", "Faile during scratchpad read");
		return;
	}
}

double ds18b20::get_temperature(bool refresh) {
	double tmpval = 0.0;

	if (refresh)
		convert_temperature();

	update_scratchpad();

	int intval = scratchpad.regs.temperature[0] | (scratchpad.regs.temperature[1] << 8);
	tmpval = (double)intval / 16.0;
	return tmpval;
}

void ds18b20::print_scratchpad() {
	ESP_LOGI("ds18b20", "temperature[0]: %02X", scratchpad.regs.temperature[0]);
	ESP_LOGI("ds18b20", "temperature[1]: %02X", scratchpad.regs.temperature[1]);
	ESP_LOGI("ds18b20", "trigger_high:   %02X", scratchpad.regs.trigger_high);
	ESP_LOGI("ds18b20", "trigger_low:    %02X", scratchpad.regs.trigger_low);
	ESP_LOGI("ds18b20", "configuration   %02X", scratchpad.regs.configuration);
	ESP_LOGI("ds18b20", "reserved[0]:    %02X", scratchpad.regs.reserved[0]);
	ESP_LOGI("ds18b20", "reserved[0]:    %02X", scratchpad.regs.reserved[1]);
	ESP_LOGI("ds18b20", "reserved[0]:    %02X", scratchpad.regs.reserved[2]);
	ESP_LOGI("ds18b20", "crc:            %02X", scratchpad.regs.crc);
}
