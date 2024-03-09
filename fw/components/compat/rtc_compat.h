#ifndef __RTC_COMPAT_H__
#define __RTC_COMPAT_H__

#include "system_info.h"

enum {
    RTC_WAKEUP_REASON_NONE = 0,
    RTC_WAKEUP_REASON_UPDATE,
    RTC_WAKEUP_REASON_ALARM,
};

int rtc_compat_init(void);
int rtc_compat_reset(void);
int rtc_compat_start(void);
int rtc_compat_stop(void);
int rtc_compat_set_running_mark(void);
bool rtc_compat_check_running_mark(void);
int rtc_compat_set_datetime(struct date_time *dt);
int rtc_compat_get_datetime(struct date_time *dt);
int rtc_compat_enable_update_intr(void);
int rtc_compat_disable_update_intr(void);
int rtc_compat_enable_alarm_intr(void);
int rtc_compat_disable_alarm_intr(void);
int rtc_compat_set_alarm(uint8_t hour, uint8_t minute);
int rtc_compat_get_wakeup_reason(int *reason);

#endif
