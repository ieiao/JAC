#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "hal/adc_types.h"
#include "system_info.h"
#include "io_compat.h"

#define TAG "ADC"

int read_battery_voltage(struct system_info *info)
{
    adc_oneshot_unit_handle_t adc1_handle;
    esp_err_t ret;

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ret = adc_oneshot_new_unit(&init_config1, &adc1_handle);
    if (ret) {
        ESP_LOGE(TAG, "adc_oneshot_new_unit");
        goto out;
    }

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_0,
    };
    ret = adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &config);
    if (ret) {
        ESP_LOGE(TAG, "adc_oneshot_config_channel");
        goto out;
    }

    adc_cali_handle_t cali_handle = NULL;
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .chan = ADC_CHANNEL_3,
        .atten = ADC_ATTEN_DB_0,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle);
    if (ret) {
        ESP_LOGE(TAG, "adc_cali_create_scheme_curve_fitting");
        goto out;
    }

    int raw[4];
    int voltage;
    uint8_t percent = 0;

    gpio_set_direction(ADC_CTL_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(ADC_CTL_GPIO, 1);
    ret = adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &raw[0]);
    ret |= adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &raw[1]);
    ret |= adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &raw[2]);
    ret |= adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &raw[3]);
    gpio_set_level(ADC_CTL_GPIO, 0);
    if (ret) {
        ESP_LOGE(TAG, "adc_oneshot_read");
        goto out;
    }

    voltage = ((raw[0] + raw[1] + raw[2] + raw[3]) / 4);
    ret = adc_cali_raw_to_voltage(cali_handle, voltage, &voltage);
    if (ret) {
        ESP_LOGE(TAG, "adc_cali_raw_to_voltage");
        goto out;
    }
    voltage *= 6.1;

    if (voltage < 3670) {
        percent = 0;
    } else {
        if (voltage >= 4100)
            percent = 100;
        else if (voltage >= 4060)
            percent = 90;
        else if (voltage >= 3980)
            percent = 80;
        else if (voltage >= 3920)
            percent = 70;
        else if (voltage >= 3870)
            percent = 60;
        else if (voltage >= 3820)
            percent = 50;
        else if (voltage >= 3790)
            percent = 40;
        else if (voltage >= 3770)
            percent = 30;
        else if (voltage >= 3740)
            percent = 20;
        else if (voltage >= 3690)
            percent = 10;
    }

    info->voltage = voltage;
    info->batt_level = percent;

    ret = adc_oneshot_del_unit(adc1_handle);
    if (ret) {
        ESP_LOGE(TAG, "adc_oneshot_del_unit");
        goto out;
    }

    ret = adc_cali_delete_scheme_curve_fitting(cali_handle);
    if (ret)
        ESP_LOGE(TAG, "adc_cali_delete_scheme_curve_fitting");

out:
    return ret;
}
