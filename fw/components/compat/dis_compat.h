#ifndef __DISPLAY_COMPAT_H__
#define __DISPLAY_COMPAT_H__

#include "system_info.h"

void dis_prepare(struct system_info *info);
void dis_destory(void);
void dis_refresh(struct system_info *info, uint8_t mode);
void dis_setpowersave(struct system_info *info, uint8_t mode);

enum {
    DIS_WAIT_IDLE_MODE_DEFAULT = 0,
    DIS_WAIT_IDLE_MODE_LOOP,
};

void dis_wait_idle(int mode);

#endif
