#ifndef __IO_COMPAT_H__
#define __IO_COMPAT_H__

#include "driver/gpio.h"

#define CS_GPIO         GPIO_NUM_10
#define SCLK_GPIO       GPIO_NUM_6
#define SDIN_GPIO       GPIO_NUM_7
#define BUSY_GPIO       GPIO_NUM_19
#define DC_GPIO         GPIO_NUM_9
#define RESET_GPIO      GPIO_NUM_8

#define I2C_SCL_GPIO    GPIO_NUM_4
#define I2C_SDA_GPIO    GPIO_NUM_5

#define BUTTON_GPIO     GPIO_NUM_0
#define CHRG_GPIO       GPIO_NUM_1
#define RTC_WAKE_GPIO   GPIO_NUM_2

#define ADC_GPIO        GPIO_NUM_3
#define ADC_CTL_GPIO    GPIO_NUM_18

void gpio_prepare(void);
void gpio_prepare_deep_sleep(struct system_info *info);
void check_battery_state(struct system_info *info);

#endif
