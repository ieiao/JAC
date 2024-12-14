#ifndef __CW2015_H__
#define __CW2015_H__

#include <stdint.h>
#include <stdbool.h>

int cw2015_init(void);
int cw2015_read_version(uint8_t *v);
int cw2015_read_soc(uint8_t *soc);
int cw2015_read_alrt(uint8_t *alrt);
int cw2015_read_athd(uint8_t *athd);
int cw2015_write_athd(uint8_t athd);
int cw2015_read_ufg(uint8_t *ufg);
int cw2015_wakeup(void);
int cw2015_deepsleep(void);
int cw2015_qstart(void);
int cw2015_restart(void);
bool is_cw2015_sleep(void);

#endif
