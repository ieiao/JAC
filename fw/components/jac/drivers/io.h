#ifndef __IO_COMPAT_H__
#define __IO_COMPAT_H__

#include "jac.h"

#define CS_GPIO         GPIO_NUM_10
#define SCLK_GPIO       GPIO_NUM_6
#define SDIN_GPIO       GPIO_NUM_7
#define BUSY_GPIO       GPIO_NUM_5
#define DC_GPIO         GPIO_NUM_9
#define RESET_GPIO      GPIO_NUM_4
#define PC_GPIO         GPIO_NUM_8

#define I2C_SCL_GPIO    GPIO_NUM_19
#define I2C_SDA_GPIO    GPIO_NUM_18

#define BUTTON_GPIO     GPIO_NUM_0
#define CHRG_GPIO       GPIO_NUM_1
#define RTC_WAKE_GPIO   GPIO_NUM_2
#define CW_ALRT_GPIO    GPIO_NUM_3

void pin_init(void);
void pin_deepsleep_setup(struct jac_info *info);
int read_charging_state(void);
void epdm_poweron(void);
void epdm_poweroff(void);

#endif
