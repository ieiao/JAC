#ifndef __SYSTEM_INFO_H__
#define __SYSTEM_INFO_H__

#include <stdint.h>
#include "u8g2.h"

enum {
    JOKE_WAKEUP_NONE = 0,
    JOKE_WAKEUP_BUTTON,
    JOKE_WAKEUP_CHARGE,
    JOKE_WAKEUP_RTC,
};

#define JOKE_FLAG_CHARGING          (0x0001)
#define JOKE_FLAG_SETTING           (0x0002)
#define JOKE_FLAG_POWERSAVING       (0x0004)
#define JOKE_FLAG_POWEDOWN          (0x1000)
#define JOKE_FLAG_BATTDEAD          (0x2000)
#define JOKE_FLAG_COREFAULT         (0x4000)
#define JOKE_FLAG_CONFIGURED        (0x8000)

struct date_time {
    uint16_t    year;
    uint8_t     month;
    uint8_t     day;
    uint8_t     hours;
    uint8_t     minutes;
    uint8_t     seconds;
    uint8_t     week;
};

struct system_info {
    uint8_t             wakeup_cause;
    uint16_t            flags;
    struct date_time    dt;
    uint32_t            voltage;
    uint8_t             batt_level;
    uint8_t             batt_mark;
    uint8_t             ps_begin;
    uint8_t             ps_end;
    uint8_t             mac[6];
    u8g2_t              *u8g2;
    struct simple_theme *theme;
};

enum {
    JOKE_MSG_NONE = 0,
    JOKE_MSG_FIRSTBOOT,
    JOKE_MSG_CONFIG_RELOAD,
    JOKE_MSG_RESET,
};

struct joke_msg {
    uint8_t     msg;
    uint8_t     reserved;
    uint16_t    d;
};

extern const uint8_t *u8g2_font_logisoso62_tn;
extern const uint8_t *u8g2_font_logisoso28_tn;
extern const uint8_t *u8g2_font_wqy16_t_gb2312a;
extern const uint8_t *u8g2_font_siji_t_6x10;

extern const uint8_t *res_tree_xbm;
extern const uint8_t *res_batt_dead;
extern const uint8_t *res_powersaving_p1;
extern const uint8_t *res_powersaving_p2;
extern const uint8_t *res_powersaving_p3;

#endif
