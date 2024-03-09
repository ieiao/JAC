#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "system_info.h"
#include "io_compat.h"

void gpio_prepare(void)
{
    gpio_set_direction(ADC_CTL_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(ADC_CTL_GPIO, 0);

    gpio_reset_pin(BUSY_GPIO);
    gpio_reset_pin(DC_GPIO);
    gpio_reset_pin(RESET_GPIO);
    gpio_reset_pin(CS_GPIO);
    gpio_set_direction(BUSY_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(DC_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(RESET_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(CS_GPIO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BUTTON_GPIO);
    gpio_reset_pin(CHRG_GPIO);
    gpio_reset_pin(RTC_WAKE_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(CHRG_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(RTC_WAKE_GPIO, GPIO_MODE_INPUT);
}

void gpio_prepare_deep_sleep(struct system_info *info)
{
    /*
     * BUSY_GPIO    -> GPIO19
     * DC_GPIO      -> GPIO9
     * RESET_GPIO   -> GPIO8
     * CS_GPIO      -> GPIO10
     * SCLK_GPIO    -> GPIO6
     * SDIN_GPIO    -> GPIO7
     * ADC_CTL_GPIO -> GPIO18
     *
     * These gpios will keep Hi-Z state in deep sleep mode, so
     * don't need do any setting before enter deep sleep mode.
     */

    /*
     * I2C, disable input and output,
     * already pull up by outside resistor, Not use internal pull up.
     */
    gpio_reset_pin(I2C_SCL_GPIO);
    gpio_reset_pin(I2C_SDA_GPIO);
    gpio_set_pull_mode(I2C_SCL_GPIO, GPIO_FLOATING);
    gpio_set_pull_mode(I2C_SDA_GPIO, GPIO_FLOATING);

    /* ADC, disable input and output, floating. */
    gpio_reset_pin(ADC_GPIO);
    gpio_set_pull_mode(ADC_GPIO, GPIO_FLOATING);

    /* Button, input, pullup only. */
    gpio_reset_pin(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);
    if (!(info->flags & (JOKE_FLAG_COREFAULT | JOKE_FLAG_BATTDEAD)))
        esp_deep_sleep_enable_gpio_wakeup(BIT(BUTTON_GPIO), ESP_GPIO_WAKEUP_GPIO_LOW);

    /* CHRG, input, ouside pullup. */
    gpio_reset_pin(CHRG_GPIO);
    gpio_set_direction(CHRG_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(CHRG_GPIO, GPIO_FLOATING);
    if (!(info->flags & JOKE_FLAG_COREFAULT) &&
        (info->flags & JOKE_FLAG_CONFIGURED))
        esp_deep_sleep_enable_gpio_wakeup(BIT(CHRG_GPIO),
                (info->flags & JOKE_FLAG_CHARGING) ?
                ESP_GPIO_WAKEUP_GPIO_HIGH : ESP_GPIO_WAKEUP_GPIO_LOW);

    /* RTC: IO2 already pull up by outside resistor, Not use internal pull up. */
    gpio_reset_pin(RTC_WAKE_GPIO);
    gpio_set_direction(RTC_WAKE_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RTC_WAKE_GPIO, GPIO_FLOATING);
    if ((info->flags & JOKE_FLAG_CONFIGURED) &&
        !(info->flags & (JOKE_FLAG_COREFAULT | JOKE_FLAG_BATTDEAD)))
        esp_deep_sleep_enable_gpio_wakeup(BIT(RTC_WAKE_GPIO), ESP_GPIO_WAKEUP_GPIO_LOW);
}

void check_battery_state(struct system_info *info)
{
    int l = gpio_get_level(CHRG_GPIO);

    if (!(info->flags & JOKE_FLAG_CHARGING)) {
        if (l == 0)
            info->flags |= JOKE_FLAG_CHARGING;
    } else {
        if (l == 1)
            info->flags &= ~JOKE_FLAG_CHARGING;
    }
}
