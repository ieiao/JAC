#include "i2c.h"

#define CW2015_ADDR        0x62

static i2c_master_dev_handle_t dev_handle;

int cw2015_init(void)
{
    return i2c_master_add_device(CW2015_ADDR, &dev_handle);
}

int cw2015_read_version(uint8_t *v)
{
    return i2c_register_read_byte(dev_handle, 0, v);
}

int cw2015_read_soc(uint8_t *soc)
{
    return i2c_register_read_byte(dev_handle, 0x04, soc);
}

int cw2015_read_alrt(uint8_t *alrt)
{
    int ret;
    uint8_t v;

    ret = i2c_register_read_byte(dev_handle, 0x06, &v);
    if (ret)
        goto out;

    *alrt = v & 0x80;

    if (v & 0x80)
        ret = i2c_register_write_byte(dev_handle, 0x06, v & 0x7f);
out:
    return ret;
}

int cw2015_read_athd(uint8_t *athd)
{
    int ret = i2c_register_read_byte(dev_handle, 0x08, athd);
    *athd >>= 3;
    return ret;
}

int cw2015_write_athd(uint8_t athd)
{
    return i2c_register_write_byte(dev_handle, 0x08, (athd << 3));
}

int cw2015_read_ufg(uint8_t *ufg)
{
    return i2c_register_read_byte(dev_handle, 0x08, ufg);
}

int cw2015_wakeup(void)
{
    return i2c_register_write_byte(dev_handle, 0x0a, 0);
}

int cw2015_deepsleep(void)
{
    return i2c_register_write_byte(dev_handle, 0x0a, 0xc0);
}

int cw2015_qstart(void)
{
    return i2c_register_write_byte(dev_handle, 0x0a, 0x30);
}

int cw2015_restart(void)
{
    return i2c_register_write_byte(dev_handle, 0x0a, 0x0f);
}

bool is_cw2015_sleep(void)
{
    uint8_t v;

    i2c_register_read_byte(dev_handle, 0, &v);

    if (v == 0)
        return true;
    else
        return false;
}
