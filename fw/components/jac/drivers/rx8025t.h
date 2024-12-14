#ifndef __RX8025T_H__
#define __RX8025T_H__

#include "jac.h"

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
int rx8025t_read_flag(uint8_t *flag);
int rx8025t_read_ext(uint8_t *ext);
int rx8025t_clear_uie_flag(void);
int rx8025t_enable_update_intr(void);
int rx8025t_disable_update_intr(void);
int rx8025t_clear_aie_flag(void);
int rx8025t_enable_alarm_intr(void);
int rx8025t_disable_alarm_intr(void);
int rx8025t_set_alarm(uint8_t hour, uint8_t minute);
int rx8025t_init(void);
int rx8025t_reset(void);
int rx8025t_set_mark(void);
bool is_rx8025t_configurated(void);

#endif
