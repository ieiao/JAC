#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "esp_err.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "lwip/inet.h"
#include "nvs_flash.h"
#include "io.h"
#include "i2c.h"
#include "u8g2.h"
#include "display.h"
#include "resources.h"
#include "http_server.h"
#include "esp_spiffs.h"
#include "rx8025t.h"
#include "cw2015.h"
#include "config.h"
#include "simple_theme.h"
#include "jac.h"

static const char *TAG = "JAC";

RTC_DATA_ATTR struct jac_info info;
TaskHandle_t config_task;
u8g2_t u8g2;
extern struct simple_theme theme_default;

uint8_t get_wakeup_cause(void)
{
    uint8_t cause = JAC_WAKEUP_NONE;
    uint64_t iomask;

    if (esp_reset_reason() == ESP_RST_DEEPSLEEP) {
        if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO) {
            iomask = esp_sleep_get_gpio_wakeup_status();
            if (iomask & BIT(BUTTON_GPIO))
                cause = JAC_WAKEUP_BUTTON;
            else if (iomask & BIT(CHRG_GPIO))
                cause = JAC_WAKEUP_CHARGE;
            else if (iomask & BIT(RTC_WAKE_GPIO))
                cause = JAC_WAKEUP_RTC;
            else if (iomask & BIT(CW_ALRT_GPIO))
                cause = JAC_WAKEUP_BATT_ALRT;
        }
    }

    return cause;
}

void checking_charging_state(void)
{
    int l = read_charging_state();

    if (l == 0)
        info.flags |= JAC_FLAG_CHARGING;
    else
        info.flags &= ~JAC_FLAG_CHARGING;
}

void jac_init()
{
    info.u8g2 = &u8g2;
    info.theme = &theme_default;

    pin_init();
    i2c_master_init();
    resources_init();
    rx8025t_init();
    cw2015_init();
    display_init(info.u8g2);
}

void jac_status_checking(void)
{
    checking_charging_state();
    info.wakeup_cause = get_wakeup_cause();

    /* Load config when first boot or after flash. */
    if (!(info.flags & JAC_FLAG_CONFIGURED) || info.wakeup_cause == JAC_WAKEUP_NONE)
        config_load(&info);
}

void jac_deep_sleep(void)
{
    display_wait_idle(WAIT_IDLE_MODE_DEFAULT);
    display_powersave(info.u8g2, 2);
    pin_deepsleep_setup(&info);
    ESP_LOGI(TAG, "Enter deep sleep");
    esp_deep_sleep_start();
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

esp_vfs_spiffs_conf_t conf = {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 10,
    .format_if_mount_failed = true
};

static void jac_config_task(void *pvParameters)
{
    while(1) {
        uint32_t v = ulTaskNotifyTake(pdTRUE, 0);

        /* Set time&date */
        if (v & 0x01) {
            info.year_add_value = info.dt.year - (info.dt.year % 100);
            rx8025t_set_datetime(&info.dt);
            rx8025t_set_mark();
            rx8025t_enable_update_intr();
            if (is_cw2015_sleep()) {
                cw2015_wakeup();
                vTaskDelay(pdMS_TO_TICKS(200));
                cw2015_read_soc(&info.soc);
                cw2015_write_athd(5);
            }
            config_set(KEY_CONFIGURED, 1);
            config_set(KEY_YEAR_ADD, info.year_add_value / 100);
            info.flags |= JAC_FLAG_CONFIGURED;
        }

        /* Set power saving duration */
        if (v & 0x02) {
            config_set(KEY_PS_BEGIN, info.ps_begin);
            config_set(KEY_PS_END, info.ps_end);
        }

        /* First boot */
        if (v & 0x04) {
            vTaskDelay(pdMS_TO_TICKS(500));
            memset(&info, 0, sizeof(struct jac_info));
            rx8025t_reset();
            if (!is_cw2015_sleep())
                cw2015_deepsleep();
            config_erase();
            esp_restart();
        }

        /* OTA prepare */
        if (v & 0x08) {
            resources_deinit();
            esp_vfs_spiffs_unregister(conf.partition_label);
        }

        /* Reset */
        if (v & 0x80) {
            vTaskDelay(pdMS_TO_TICKS(500));
            config_deinit();
            esp_restart();
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

int jac_config_init()
{
    xTaskCreate(jac_config_task, "config",
                configMINIMAL_STACK_SIZE + 512, NULL,
                configMAX_PRIORITIES - 1, &config_task);

    config_init();
    esp_netif_init();
    esp_event_loop_create_default();
    nvs_flash_init();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t wifi_config;
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);

    memset(&wifi_config, 0, sizeof(wifi_config_t));
    wifi_config.ap.max_connection = 1;
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    esp_read_mac(info.mac, ESP_MAC_WIFI_SOFTAP);
    sprintf((char *)&wifi_config.ap.ssid, "JAC-%02X%02X", info.mac[4], info.mac[5]);

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);

    esp_wifi_start();

    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);

    char ip_addr[16];
    inet_ntoa_r(ip_info.ip.addr, ip_addr, 16);
    ESP_LOGI(TAG, "Set up softAP with IP: %s", ip_addr);

    esp_vfs_spiffs_register(&conf);
    http_server_start();

    return 0;
}

void jac_config_process(void)
{
    int count = 0;

    jac_config_init();

    dispart(info.theme->config_wait, &info);
    display_wait_idle(WAIT_IDLE_MODE_LOOP);
    display_powersave(info.u8g2, 2);
    epdm_poweroff();

    while(1)
    {
        vTaskDelay(10000/portTICK_PERIOD_MS);
        count++;
        if (count >= 12) {
            /* 2 mins, check any device are connected. */
            wifi_sta_list_t sta_list;
            esp_wifi_ap_get_sta_list(&sta_list);
            if (sta_list.num == 0) {
                ESP_LOGI(TAG, "Timeout, resetting");
                esp_restart();
            }
        }
    }
}

bool jac_in_power_saving(void)
{
    bool ret = false;

    if (info.ps_begin == info.ps_end) {
        ret = false;
    } else if (info.ps_begin > info.ps_end) {
        if (info.dt.hours >= info.ps_begin ||
            info.dt.hours < info.ps_end)
            ret = true;
    } else if (info.ps_begin < info.ps_end) {
        if (info.dt.hours >= info.ps_begin &&
            info.dt.hours < info.ps_end)
            ret = true;
    }

    return ret;
}

void jac(void)
{
    jac_init();
    jac_status_checking();

    /* Have to check fault state after jac_core_prepare called. */
    if (info.flags & JAC_FLAG_COREFAULT) {
        dispart(info.theme->core_failure, &info);
        jac_deep_sleep();
    }

    /* Checking first boot and power lost. */
    if (!(info.flags & JAC_FLAG_CONFIGURED) ||
        !is_rx8025t_configurated()) {
        if (info.wakeup_cause == JAC_WAKEUP_BUTTON) {
            /* execute config. */
            ESP_LOGI(TAG, "Starting config process");
            jac_config_process();
        } else {
            ESP_LOGI(TAG, "JAC not configurated, sleep now");
            dispart(info.theme->welcome, &info);
            rx8025t_reset();
            if (!is_cw2015_sleep()) {
                ESP_LOGI(TAG, "CW2015 not sleep, sleep now");
                cw2015_deepsleep();
            }
            jac_deep_sleep();
        }
    } else {
        if (info.wakeup_cause == JAC_WAKEUP_BATT_ALRT) {
            uint8_t alert;
            cw2015_read_alrt(&alert);
            info.flags |= JAC_FLAG_BATTDEAD;
            rx8025t_disable_intr();
            rx8025t_clear_flag();
            dispart(info.theme->battery_dead, &info);
            jac_deep_sleep();
        } else if (info.wakeup_cause == JAC_WAKEUP_BUTTON) {
            rx8025t_get_datetime(&info.dt);
            info.dt.year += info.year_add_value;
            if (jac_in_power_saving()) {
                dispart(info.theme->primary, &info);
                rx8025t_enable_update_intr();
                jac_deep_sleep();
            } else {
                /* execute config. */
                jac_config_process();
            }
        } else {
            uint8_t flag;

            if (info.flags & JAC_FLAG_BATTDEAD &&
                info.wakeup_cause == JAC_WAKEUP_CHARGE)
                info.flags &= ~JAC_FLAG_BATTDEAD;

            rx8025t_read_flag(&flag);
            rx8025t_clear_flag();
            rx8025t_disable_intr();

            rx8025t_get_datetime(&info.dt);
            info.dt.year += info.year_add_value;

            /* Read SOC immediately after flash, OTA and charger removed. */
            if (info.soc == 0 || info.wakeup_cause == JAC_WAKEUP_CHARGE)
                cw2015_read_soc(&info.soc);

            /* Read SOC every 30 minutes */
            if ((info.dt.minutes % 30) == 0) {
                uint8_t soc;
                cw2015_read_soc(&soc);
                /* Only allow new soc greater than old soc when it's charging. */
                if (soc > info.soc && info.flags & JAC_FLAG_CHARGING)
                    info.soc = soc;
            }

            if (jac_in_power_saving()) {
                rx8025t_disable_update_intr();
                rx8025t_set_alarm(info.ps_end, 0);
                rx8025t_enable_alarm_intr();
                dispart(info.theme->powersaving, &info);
            } else {
                rx8025t_disable_alarm_intr();
                rx8025t_enable_update_intr();
                dispart(info.theme->primary, &info);
            }

            jac_deep_sleep();
        }
    }
}
