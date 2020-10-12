/*
 * i2cbus.cpp
 *
 *  Created on: Sep 26, 2020
 *      Author: wit
 */

#include "i2cbus.h"

#include <memory>


#define I2C_TX_BUF_DISABLE  	0                /*!< I2C master do not need buffer */
#define I2C_RX_BUF_DISABLE  	0                /*!< I2C master do not need buffer */

#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */

i2cbus::i2cbus(int i2c_port, gpio_num_t i2c_sda, gpio_num_t i2c_scl, int freq) {
	i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = i2c_sda;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = i2c_scl;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = freq;

    i2c_param_config(i2c_port, &conf);
    i2c_driver_install(i2c_port, conf.mode, I2C_RX_BUF_DISABLE, I2C_TX_BUF_DISABLE, 0);
    i2c_bus_number = i2c_port;
}

i2cbus::~i2cbus() {
	// TODO Auto-generated destructor stub
}


void i2cbus::i2c_write_reg(uint8_t i2c_addr, uint8_t reg, uint8_t byte) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    uint8_t addr = i2c_addr << 1;
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, addr | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_write(cmd, &byte, 1, ACK_CHECK_EN);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(this->i2c_bus_number, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
		throw std::runtime_error("I2C: Failed to write byte");
}
std::vector<uint8_t> * i2cbus::i2c_read_regs(uint8_t i2c_addr, uint8_t reg, int count) {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    uint8_t addr = i2c_addr << 1;
    uint8_t *data = new uint8_t[count];
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, addr | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
	i2c_master_start(cmd);

	i2c_master_write_byte(cmd, addr| I2C_MASTER_READ, ACK_CHECK_EN);
	i2c_master_read(cmd, data, count, I2C_MASTER_LAST_NACK);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(this->i2c_bus_number, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	if (ret != ESP_OK)
		throw std::runtime_error("I2C: Failed to read byte");

	std::vector<uint8_t> *v = new std::vector<uint8_t>(data, data+count);
	delete [] data;

	return v;
}
