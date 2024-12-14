#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>
#include "driver/i2c_master.h"

int i2c_master_init(void);
int i2c_master_add_device(uint8_t slave_addr, i2c_master_dev_handle_t *dev_handle);
int i2c_register_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data);
int i2c_register_write_bytes(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len);
int i2c_register_read_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data);
int i2c_register_read_bytes(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len);

#endif
