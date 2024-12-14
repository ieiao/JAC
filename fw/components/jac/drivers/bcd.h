#ifndef __BCD_H__
#define __BCD_H__

unsigned char bcd2bin(unsigned char val)
{
    return (val & 0x0f) + (val >> 4) * 10;
}

unsigned char bin2bcd(unsigned char val)
{
    return ((val / 10) << 4) + val % 10;
}

#endif
