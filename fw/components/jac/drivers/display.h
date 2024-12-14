#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <stdint.h>
#include "u8g2.h"

void display_init(u8g2_t *u8g2);
void display_refresh(u8g2_t *u8g2, uint8_t mode);
void display_powersave(u8g2_t *u8g2, uint8_t mode);

enum {
    WAIT_IDLE_MODE_DEFAULT = 0,
    WAIT_IDLE_MODE_LOOP,
};

void display_wait_idle(int mode);

#endif
