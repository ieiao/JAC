#include <stdbool.h>

#include "rtc_compat.h"
#include "esp_log.h"
#include "system_info.h"
#include "rx8025t.h"

#define RTC_RUNNING_MARK        0xaa

int rtc_compat_init(void)
{
    return rx8025t_init();
}

int rtc_compat_reset(void)
{
    return rx8025t_reset();
}

int rtc_compat_start(void)
{
    return rx8025t_start();
}

int rtc_compat_stop(void)
{
    return rx8025t_stop();
}

int rtc_compat_set_running_mark(void)
{
    return rx8025t_set_ram(RTC_RUNNING_MARK);
}

bool rtc_compat_check_running_mark(void)
{
    int ret;
    uint8_t mark;

    ret = rx8025t_get_ram(&mark);
    if (ret)
        return false;

    if (mark != RTC_RUNNING_MARK)
        return false;

    return true;
}

int rtc_compat_set_datetime(struct date_time *dt)
{
    return rx8025t_set_datetime(dt);
}

int rtc_compat_get_datetime(struct date_time *dt)
{
    return rx8025t_get_datetime(dt);
}

int rtc_compat_enable_update_intr(void)
{
    return rx8025t_enable_update_intr();
}

int rtc_compat_disable_update_intr(void)
{
    return rx8025t_disable_update_intr();
}

int rtc_compat_enable_alarm_intr(void)
{
    return rx8025t_enable_alarm_intr();
}

int rtc_compat_disable_alarm_intr(void)
{
    return rx8025t_disable_alarm_intr();
}

int rtc_compat_set_alarm(uint8_t hour, uint8_t minute)
{
    return rx8025t_set_alarm(hour, minute);
}

int rtc_compat_get_wakeup_reason(int *reason)
{
    uint8_t flags;
    int ret;

    flags = rx8025t_read_flag();
    if (flags == 0xff)
        return -1;

    ret = rx8025t_clear_flag();
    if (ret)
        return ret;

    if ((flags & RX8025T_FLAG_UF) != 0)
        *reason = RTC_WAKEUP_REASON_UPDATE;
    else if((flags & RX8025T_FLAG_AF) != 0)
        *reason = RTC_WAKEUP_REASON_ALARM;
    else
        *reason = RTC_WAKEUP_REASON_NONE;

    return 0;
}
