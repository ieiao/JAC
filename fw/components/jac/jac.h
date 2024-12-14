#ifndef __JAC_H__
#define __JAC_H__

#include <stdint.h>
#include "u8g2.h"

enum {
    JAC_WAKEUP_NONE = 0,
    JAC_WAKEUP_BUTTON,
    JAC_WAKEUP_CHARGE,
    JAC_WAKEUP_RTC,
    JAC_WAKEUP_BATT_ALRT,
};

#define JAC_FLAG_CHARGING          (0x0001)
#define JAC_FLAG_SETTING           (0x0002)
#define JAC_FLAG_POWERSAVING       (0x0004)
#define JAC_FLAG_POWEDOWN          (0x1000)
#define JAC_FLAG_BATTDEAD          (0x2000)
#define JAC_FLAG_COREFAULT         (0x4000)
#define JAC_FLAG_CONFIGURED        (0x8000)

struct date_time {
    uint16_t    year;
    uint8_t     month;
    uint8_t     day;
    uint8_t     hours;
    uint8_t     minutes;
    uint8_t     seconds;
    uint8_t     week;
};

struct jac_info {
    uint8_t             wakeup_cause;
    uint16_t            flags;
    struct date_time    dt;
    uint16_t            year_add_value;
    uint32_t            voltage;
    uint8_t             soc;
    uint8_t             batt_mark;
    uint8_t             ps_begin;
    uint8_t             ps_end;
    uint8_t             mac[6];
    u8g2_t              *u8g2;
    struct simple_theme *theme;
};

enum {
    JAC_MSG_NONE = 0,
    JAC_MSG_FIRSTBOOT,
    JAC_MSG_CONFIG_RELOAD,
    JAC_MSG_RESET,
};

struct jac_msg {
    uint8_t     msg;
    uint8_t     reserved;
    uint16_t    d;
};

#endif
