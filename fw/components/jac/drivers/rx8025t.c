#include <stdbool.h>

#include "rx8025t.h"
#include "i2c.h"
#include "bcd.h"

#define RX8025T_ADDR        0x32

#define SEC_REGISTER        0x00
#define RAM_REGISTER        0x07
#define MIN_ALARM_REG       0x08
#define EXT_REGISTER        0x0d
#define FLAG_REGISTER       0x0e
#define CTRL_REGISTER       0x0f

#define RUNNING_MARK        0xaa

static i2c_master_dev_handle_t dev_handle;

int rx8025t_stop(void)
{
    uint8_t v;
    int ret;

    ret = i2c_register_read_byte(dev_handle, CTRL_REGISTER, &v);
    if (ret)
        goto out;

    v |= 0x01;
    ret = i2c_register_write_byte(dev_handle, CTRL_REGISTER, v);
out:
    return ret;
}

int rx8025t_start(void)
{
    uint8_t v;
    int ret;

    ret = i2c_register_read_byte(dev_handle, CTRL_REGISTER, &v);
    if (ret)
        goto out;

    v &= ~0x01;
    ret = i2c_register_write_byte(dev_handle, CTRL_REGISTER, v);
out:
    return ret;
}

int rx8025t_set_datetime(struct date_time *dt)
{
    uint8_t datetime[7];

    datetime[0] = bin2bcd(dt->seconds);
    datetime[1] = bin2bcd(dt->minutes);
    datetime[2] = bin2bcd(dt->hours);
    datetime[4] = bin2bcd(dt->day);
    datetime[5] = bin2bcd(dt->month);
    datetime[6] = bin2bcd(dt->year % 100);

    if (dt->week == 7)
        datetime[3] = 0x01;
    else
        datetime[3] = 0x01 << dt->week;

    return i2c_register_write_bytes(dev_handle, SEC_REGISTER, datetime, sizeof(datetime));
}

int rx8025t_get_datetime(struct date_time *dt)
{
    uint8_t datetime[7];
    int ret;

    ret = i2c_register_read_bytes(dev_handle, SEC_REGISTER, datetime, sizeof(datetime));
    if (ret)
        return ret;

    dt->seconds = bcd2bin(datetime[0]);
    dt->minutes = bcd2bin(datetime[1]);
    dt->hours   = bcd2bin(datetime[2]);
    dt->day     = bcd2bin(datetime[4]);
    dt->month   = bcd2bin(datetime[5]);
    dt->year    = bcd2bin(datetime[6]);

    switch (datetime[3]) {
        case 0x01: dt->week = 7; break;
        case 0x02: dt->week = 1; break;
        case 0x04: dt->week = 2; break;
        case 0x08: dt->week = 3; break;
        case 0x10: dt->week = 4; break;
        case 0x20: dt->week = 5; break;
        case 0x40: dt->week = 6; break;
        default:
            break;
    }

    return 0;
}

int rx8025t_set_ram(uint8_t v)
{
    return i2c_register_write_byte(dev_handle, RAM_REGISTER, v);
}

int rx8025t_get_ram(uint8_t *v)
{
    return i2c_register_read_byte(dev_handle, RAM_REGISTER, v);
}

int rx8025t_disable_intr(void)
{
    uint8_t v;
    int ret;

    /* Disable UIE & TIE & AIE. */
    ret = i2c_register_read_byte(dev_handle, CTRL_REGISTER, &v);
    if (ret)
        goto out;

    v &= ~0x38;
    ret = i2c_register_write_byte(dev_handle, CTRL_REGISTER, v);
out:
    return ret;
}

int rx8025t_clear_flag(void)
{
    uint8_t v;
    int ret;

    ret = i2c_register_read_byte(dev_handle, FLAG_REGISTER, &v);
    if (ret)
        goto out;

    v &= ~0x3b;
    ret = i2c_register_write_byte(dev_handle, FLAG_REGISTER, v);
out:
    return ret;
}

int rx8025t_read_flag(uint8_t *flag)
{
    int ret;

    ret = i2c_register_read_byte(dev_handle, FLAG_REGISTER, flag);
    if (ret)
        goto out;

    *flag &= 0x38;
out:
    return ret;
}

int rx8025t_read_ext(uint8_t *ext)
{
    return i2c_register_read_byte(dev_handle, EXT_REGISTER, ext);
}

int rx8025t_clear_uie_flag(void)
{
    uint8_t v;
    int ret;

    ret = i2c_register_read_byte(dev_handle, FLAG_REGISTER, &v);
    if (ret)
        goto out;

    v &= ~0x20;
    ret = i2c_register_write_byte(dev_handle, FLAG_REGISTER, v);
out:
    return ret;
}

int rx8025t_enable_update_intr(void)
{
    uint8_t v;
    int ret;

    /* Clear UF first. */
    ret = rx8025t_clear_uie_flag();
    if (ret)
        goto out;

    /* Set minute update interrupt. */
    ret = i2c_register_read_byte(dev_handle, EXT_REGISTER, &v);
    if (ret)
        goto out;

    v |= 0x20;
    ret = i2c_register_write_byte(dev_handle, EXT_REGISTER, v);
    if (ret)
        goto out;

    /* Enable UIE. */
    ret = i2c_register_read_byte(dev_handle, CTRL_REGISTER, &v);
    if (ret)
        goto out;

    v |= 0x20;
    ret = i2c_register_write_byte(dev_handle, CTRL_REGISTER, v);
out:
    return ret;
}

int rx8025t_disable_update_intr(void)
{
    uint8_t v;
    int ret;

    /* Disable UIE. */
    ret = i2c_register_read_byte(dev_handle, CTRL_REGISTER, &v);
    if (ret)
        goto out;

    v &= ~0x20;
    ret = i2c_register_write_byte(dev_handle, CTRL_REGISTER, v);
out:
    return ret;
}

int rx8025t_clear_aie_flag(void)
{
    uint8_t v;
    int ret;

    ret = i2c_register_read_byte(dev_handle, FLAG_REGISTER, &v);
    if (ret)
        goto out;

    v &= ~0x08;
    ret = i2c_register_write_byte(dev_handle, FLAG_REGISTER, v);
out:
    return ret;
}

int rx8025t_enable_alarm_intr(void)
{
    uint8_t v;
    int ret;

    /* Clear AF first. */
    ret = rx8025t_clear_aie_flag();
    if (ret)
        goto out;

    /* Set Alarm. */
    ret = i2c_register_read_byte(dev_handle, EXT_REGISTER, &v);
    if (ret)
        goto out;

    v |= 0x40;
    ret = i2c_register_write_byte(dev_handle, EXT_REGISTER, v);
    if (ret)
        goto out;

    /* Enable AIE. */
    ret = i2c_register_read_byte(dev_handle, CTRL_REGISTER, &v);
    if (ret)
        goto out;

    v |= 0x08;
    ret = i2c_register_write_byte(dev_handle, CTRL_REGISTER, v);
out:
    return ret;
}

int rx8025t_disable_alarm_intr(void)
{
    uint8_t v;
    int ret;

    /* Disable AIE. */
    ret = i2c_register_read_byte(dev_handle, CTRL_REGISTER, &v);
    if (ret)
        goto out;

    v &= ~0x08;
    ret = i2c_register_write_byte(dev_handle, CTRL_REGISTER, v);
out:
    return ret;
}

int rx8025t_set_alarm(uint8_t hour, uint8_t minute)
{
    int ret;
    uint8_t d[3];

    d[0] = bin2bcd(minute);
    d[1] = bin2bcd(hour);
    d[2] = 0x80;

    ret = i2c_register_write_bytes(dev_handle, MIN_ALARM_REG, d, 3);
    return ret;
}

int rx8025t_init(void)
{
    return i2c_master_add_device(RX8025T_ADDR, &dev_handle);
}

int rx8025t_reset(void)
{
    uint8_t v;
    int ret;

    /* Disable timer. */
    ret = i2c_register_read_byte(dev_handle, EXT_REGISTER, &v);
    if (ret)
        goto out;

    v &= ~0x10;
    ret = i2c_register_write_byte(dev_handle, EXT_REGISTER, v);
    if (ret)
        goto out;

    ret = rx8025t_disable_intr();
    if (ret)
        goto out;

    ret = rx8025t_clear_flag();
    if (ret)
        goto out;

    ret = rx8025t_set_ram(0);
    if (ret)
        goto out;

    ret = rx8025t_stop();
out:
    return ret;
}

int rx8025t_set_mark(void)
{
    return rx8025t_set_ram(RUNNING_MARK);
}

bool is_rx8025t_configurated(void)
{
    int ret;
    uint8_t mark;

    ret = rx8025t_get_ram(&mark);
    if (ret)
        return false;

    if (mark != RUNNING_MARK)
        return false;

    return true;
}


