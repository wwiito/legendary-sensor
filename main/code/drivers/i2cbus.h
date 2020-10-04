/*
 * i2cbus.h
 *
 *  Created on: Sep 26, 2020
 *      Author: wit
 */

#ifndef MAIN_CODE_DRIVERS_I2CBUS_H_
#define MAIN_CODE_DRIVERS_I2CBUS_H_

#include "driver/gpio.h"
#include "driver/i2c.h"

#include <memory>
#include <vector>

class i2cbus {
public:
	i2cbus(int i2c_port, gpio_num_t i2c_sda, gpio_num_t i2c_scl, int freq = 100000);
	virtual ~i2cbus();

	void i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t byte);
	std::vector<uint8_t> * i2c_read_regs(uint8_t i2c_addr, uint8_t reg, int count);


private:
	int i2c_bus_number;
	int i2c_bus_timeout = 1000 / portTICK_RATE_MS;
};

#endif /* MAIN_CODE_DRIVERS_I2CBUS_H_ */
