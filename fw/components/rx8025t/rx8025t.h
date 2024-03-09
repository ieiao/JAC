#ifndef __RX8025T_H__
#define __RX8025T_H__

#include "system_info.h"

#define RX8025T_FLAG_UF     0x20
#define RX8025T_FLAG_TF     0x10
#define RX8025T_FLAG_AF     0x08

int rx8025t_stop(void);
int rx8025t_start(void);
int rx8025t_set_datetime(struct date_time *dt);
int rx8025t_get_datetime(struct date_time *dt);
int rx8025t_set_ram(uint8_t v);
int rx8025t_get_ram(uint8_t *v);
int rx8025t_disable_intr(void);
int rx8025t_clear_flag(void);
uint8_t rx8025t_read_flag(void);
uint8_t rx8025t_read_ext(void);
int rx8025t_clear_uie_flag(void);
int rx8025t_enable_update_intr(void);
int rx8025t_disable_update_intr(void);
int rx8025t_clear_aie_flag(void);
int rx8025t_enable_alarm_intr(void);
int rx8025t_disable_alarm_intr(void);
int rx8025t_set_alarm(uint8_t hour, uint8_t minute);
int rx8025t_init(void);
int rx8025t_reset(void);

#endif
