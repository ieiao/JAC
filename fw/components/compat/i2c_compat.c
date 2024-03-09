#include "freertos/portmacro.h"
#include "esp_err.h"

#include "system_info.h"
#include "io_compat.h"
#include "i2c_compat.h"

#include <string.h>

#define I2C_MASTER_NUM              0
#define I2C_MASTER_FREQ_HZ          400000
#define I2C_MASTER_TIMEOUT_MS       1000

i2c_master_bus_handle_t bus_handle;

int i2c_master_init(void)
{
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = I2C_SCL_GPIO,
        .sda_io_num = I2C_SDA_GPIO,
        .glitch_ignore_cnt = 7,
    };

    return i2c_new_master_bus(&i2c_mst_config, &bus_handle);
}

int i2c_master_add_device(uint8_t slave_addr, i2c_master_dev_handle_t *dev_handle)
{
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = slave_addr,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    return i2c_master_bus_add_device(bus_handle, &dev_cfg, dev_handle);
}

int i2c_register_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};

    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

int i2c_register_write_bytes(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len)
{
    uint8_t write_buf[17];

    if (len > 16)
        return ESP_ERR_INVALID_ARG;

    write_buf[0] = reg_addr;
    memcpy(&write_buf[1], data, len);

    return i2c_master_transmit(dev_handle, write_buf, len + 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

int i2c_register_read_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data)
{
    return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, data, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

int i2c_register_read_bytes(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}
