#include "driver/gpio.h"
#include "esp_sleep.h"
#include "io.h"

void pin_init(void)
{
    gpio_reset_pin(BUSY_GPIO);
    gpio_reset_pin(DC_GPIO);
    gpio_reset_pin(RESET_GPIO);
    gpio_reset_pin(CS_GPIO);

    gpio_set_direction(BUSY_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUSY_GPIO, GPIO_PULLDOWN_ONLY);

    gpio_set_direction(DC_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(RESET_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(CS_GPIO, GPIO_MODE_OUTPUT);

    gpio_set_direction(PC_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(PC_GPIO, GPIO_FLOATING);

    gpio_reset_pin(BUTTON_GPIO);
    gpio_reset_pin(CHRG_GPIO);
    gpio_reset_pin(RTC_WAKE_GPIO);
    gpio_reset_pin(CW_ALRT_GPIO);

    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);

    gpio_set_direction(CHRG_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(CHRG_GPIO, GPIO_FLOATING);

    gpio_set_direction(RTC_WAKE_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RTC_WAKE_GPIO, GPIO_FLOATING);

    /* Enable EPDM power supply */
    gpio_set_level(PC_GPIO, 0);
}

void epdm_poweron(void)
{
    gpio_set_level(PC_GPIO, 0);
}

void epdm_poweroff(void)
{
    gpio_set_level(PC_GPIO, 1);
}

void pin_deepsleep_setup(struct jac_info *info)
{
    gpio_set_direction(DC_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(RESET_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(CS_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(SCLK_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(SDIN_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(BUSY_GPIO, GPIO_MODE_INPUT);

    gpio_set_pull_mode(DC_GPIO, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(RESET_GPIO, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(CS_GPIO, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SCLK_GPIO, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SDIN_GPIO, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(BUSY_GPIO, GPIO_PULLUP_ONLY);

    gpio_set_direction(PC_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PC_GPIO, GPIO_FLOATING);

    /*
     * I2C, disable input and output,
     * already pull up by outside resistor, Not use internal pull up.
     */
    gpio_set_direction(I2C_SCL_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(I2C_SDA_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(I2C_SCL_GPIO, GPIO_FLOATING);
    gpio_set_pull_mode(I2C_SDA_GPIO, GPIO_FLOATING);

    /* Button, input, pullup only. */
    gpio_reset_pin(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);
    if (!(info->flags & (JAC_FLAG_COREFAULT | JAC_FLAG_BATTDEAD)))
        esp_deep_sleep_enable_gpio_wakeup(BIT(BUTTON_GPIO), ESP_GPIO_WAKEUP_GPIO_LOW);

    /* CHRG, input, ouside pullup. */
    gpio_reset_pin(CHRG_GPIO);
    gpio_set_direction(CHRG_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(CHRG_GPIO, GPIO_FLOATING);
    if (!(info->flags & JAC_FLAG_COREFAULT) &&
        (info->flags & JAC_FLAG_CONFIGURED))
        esp_deep_sleep_enable_gpio_wakeup(BIT(CHRG_GPIO),
                (info->flags & JAC_FLAG_CHARGING) ?
                ESP_GPIO_WAKEUP_GPIO_HIGH : ESP_GPIO_WAKEUP_GPIO_LOW);

    /* RTC: IO2 already pull up by outside resistor, Not use internal pull up. */
    gpio_reset_pin(RTC_WAKE_GPIO);
    gpio_set_direction(RTC_WAKE_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(RTC_WAKE_GPIO, GPIO_FLOATING);
    if ((info->flags & JAC_FLAG_CONFIGURED) &&
        !(info->flags & (JAC_FLAG_COREFAULT | JAC_FLAG_BATTDEAD)))
        esp_deep_sleep_enable_gpio_wakeup(BIT(RTC_WAKE_GPIO), ESP_GPIO_WAKEUP_GPIO_LOW);

    /* CW alert gpio, input, pull up only. */
    gpio_reset_pin(CW_ALRT_GPIO);
    gpio_set_direction(CW_ALRT_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(CW_ALRT_GPIO, GPIO_PULLUP_ONLY);
    if ((info->flags & JAC_FLAG_CONFIGURED) &&
        !(info->flags & (JAC_FLAG_COREFAULT | JAC_FLAG_BATTDEAD)))
        esp_deep_sleep_enable_gpio_wakeup(BIT(CW_ALRT_GPIO), ESP_GPIO_WAKEUP_GPIO_LOW);
}

int read_charging_state(void)
{
    return gpio_get_level(CHRG_GPIO);
}
