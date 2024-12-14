#include <stdint.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_sleep.h"
#include "display.h"
#include "io.h"
#include "resources.h"
#include "jac.h"

static spi_device_handle_t spi_dev;

uint8_t u8g2_esp32_spi_byte_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch(msg) {
    case U8X8_MSG_BYTE_SET_DC:
        gpio_set_level(DC_GPIO, arg_int);
        break;

    case U8X8_MSG_BYTE_INIT: {
        esp_err_t ret;

        spi_bus_config_t bus_cfg = {
            .mosi_io_num = SDIN_GPIO,
            .miso_io_num = -1,
            .sclk_io_num = SCLK_GPIO,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 8192,
        };

        spi_device_interface_config_t dev_cfg = {
            .mode = 0,
            .clock_speed_hz = 1000000,
            .spics_io_num = -1,
            .queue_size = 64,
        };

        ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
        ESP_ERROR_CHECK(ret);

        ret = spi_bus_add_device(SPI2_HOST, &dev_cfg, &spi_dev);
        ESP_ERROR_CHECK(ret);
        break;
    }

    case U8X8_MSG_BYTE_SEND: {
        spi_transaction_t trans_desc;

        trans_desc.addr      = 0;
        trans_desc.cmd       = 0;
        trans_desc.flags     = 0;
        trans_desc.length    = 8 * arg_int; // Number of bits NOT number of bytes.
        trans_desc.rxlength  = 0;
        trans_desc.tx_buffer = arg_ptr;
        trans_desc.rx_buffer = NULL;

        while(gpio_get_level(BUSY_GPIO) == 1)
            vTaskDelay(10/portTICK_PERIOD_MS);

        ESP_ERROR_CHECK(spi_device_polling_transmit(spi_dev, &trans_desc));
        break;
    }

    case U8X8_MSG_BYTE_START_TRANSFER:
        gpio_set_level(CS_GPIO, 0);
        break;

    case U8X8_MSG_BYTE_END_TRANSFER:
        gpio_set_level(CS_GPIO, 1);
        break;

    default:
        break;
    }
    return 0;
}

uint8_t u8g2_esp32_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch(msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            break;

        case U8X8_MSG_GPIO_RESET:
                gpio_set_level(RESET_GPIO, arg_int);
            break;

        case U8X8_MSG_GPIO_CS:
                gpio_set_level(CS_GPIO, arg_int);
            break;

        case U8X8_MSG_GPIO_E:
                return (uint8_t)gpio_get_level(BUSY_GPIO);
            break;

        case U8X8_MSG_DELAY_MILLI:
            vTaskDelay(arg_int/portTICK_PERIOD_MS);
            break;

        default:
            break;
    }
    return 0;
}

void u8x8_RefreshDisplayM(u8x8_t *u8x8, uint8_t v)
{
    u8x8->display_cb(u8x8, U8X8_MSG_DISPLAY_REFRESH, v, NULL);
}

void display_init(u8g2_t *u8g2)
{
    u8g2_Setup_il3895_122x250_f(u8g2, &u8g2_cb_r1, u8g2_esp32_spi_byte_cb, u8g2_esp32_gpio_and_delay_cb);
    u8x8_InitDisplay(u8g2_GetU8x8(u8g2));
    u8g2_SetDrawColor(u8g2, 0);
    u8g2_SetFont(u8g2, u8g2_font_wqy16_t_gb2312a);
}

void display_refresh(u8g2_t *u8g2, uint8_t mode)
{
    u8g2_UpdateDisplay(u8g2);
    u8x8_RefreshDisplayM(u8g2_GetU8x8(u8g2), mode);
}

void display_powersave(u8g2_t *u8g2, uint8_t mode)
{
    u8g2_SetPowerSave(u8g2, mode);
}

/*
 * display_wait_idle - Wait unit display is idle.
 *
 * Call this function with WAIT_IDLE_MODE_DEFAULT
 * will makes MCU into light sleep mode.
 */
void display_wait_idle(int mode)
{
    if (gpio_get_level(BUSY_GPIO) == 1) {
        if (mode == WAIT_IDLE_MODE_DEFAULT) {
            gpio_hold_en(PC_GPIO);
            gpio_wakeup_enable(BUSY_GPIO, GPIO_INTR_LOW_LEVEL);
            esp_sleep_enable_gpio_wakeup();
            esp_light_sleep_start();
            gpio_wakeup_disable(BUSY_GPIO);
            gpio_hold_dis(PC_GPIO);
        } else {
            while(gpio_get_level(BUSY_GPIO) == 1)
                vTaskDelay(200/portTICK_PERIOD_MS);
        }
    }
}
