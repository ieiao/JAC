#include <stdint.h>
#include <stdio.h>
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
#include "rx8025t.h"
#include "sdkconfig.h"
#include "u8g2.h"
#include "system_info.h"
#include "simple_theme.h"
#include "io_compat.h"
#include "i2c_compat.h"
#include "dis_compat.h"
#include "rtc_compat.h"
#include "adc_compat.h"
#include "config_compat.h"
#include "resources.h"
#include "dns_server.h"
#include "http_server.h"
#include "esp_spiffs.h"

static const char *TAG = "Joke";

RTC_DATA_ATTR struct system_info sys_info;
RTC_DATA_ATTR uint8_t test;
QueueHandle_t msg_queue;
TaskHandle_t msg_task;
u8g2_t u8g2;
extern struct simple_theme theme_default;

uint8_t joke_get_wakeup_cause(void)
{
    uint8_t cause = JOKE_WAKEUP_NONE;
    uint64_t iomask;

    if (esp_reset_reason() == ESP_RST_DEEPSLEEP) {
        if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO) {
            iomask = esp_sleep_get_gpio_wakeup_status();
            if (iomask & BIT(BUTTON_GPIO))
                cause = JOKE_WAKEUP_BUTTON;
            else if (iomask & BIT(RTC_WAKE_GPIO))
                cause = JOKE_WAKEUP_RTC;
            else if (iomask & BIT(CHRG_GPIO))
                cause = JOKE_WAKEUP_CHARGE;
        }
    }

    return cause;
}

void joke_core_prepare()
{
    int ret;

    gpio_prepare();
    check_battery_state(&sys_info);
    sys_info.u8g2 = &u8g2;
    sys_info.theme = &theme_default;
    dis_prepare(&sys_info);

    sys_info.wakeup_cause = joke_get_wakeup_cause();
    /* Try load config. */
    if (!(sys_info.flags & JOKE_FLAG_CONFIGURED) ||
        sys_info.wakeup_cause == JOKE_WAKEUP_NONE) {
        ret = config_load(&sys_info);
        if (ret) {
            ESP_LOGE(TAG, "%s %d %d", __FILE__, __LINE__, ret);
            goto error_out;
        }
    }

    ret = resources_prepare();
    if (ret) {
        ESP_LOGE(TAG, "%s %d %d", __FILE__, __LINE__, ret);
        goto error_out;
    }

    ret = i2c_master_init();
    if (ret) {
        ESP_LOGE(TAG, "%s %d %d", __FILE__, __LINE__, ret);
        goto error_out;
    }

    ret = rtc_compat_init();
    if (ret) {
        ESP_LOGE(TAG, "%s %d %d", __FILE__, __LINE__, ret);
        goto error_out;
    }

    return;

error_out:
    sys_info.flags = JOKE_FLAG_COREFAULT;
    return;
}

void joke_deep_sleep(void)
{
    dis_wait_idle(DIS_WAIT_IDLE_MODE_DEFAULT);
    dis_setpowersave(&sys_info, 2);
    dis_destory();
    gpio_prepare_deep_sleep(&sys_info);
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

static void joke_msg_task(void *pvParameters)
{
    BaseType_t ret;
    QueueHandle_t q = (QueueHandle_t)pvParameters;
    struct joke_msg msg;

    while(1) {
        memset(&msg, 0, sizeof(struct joke_msg));
        ret = xQueueReceive(q, &msg, portMAX_DELAY);
        if (ret == pdTRUE) {
            switch (msg.msg) {
                case JOKE_MSG_FIRSTBOOT:
                    memset(&sys_info, 0, sizeof(struct system_info));
                    rtc_compat_reset();
                    config_erase();
                    esp_restart();
                    break;

                case JOKE_MSG_RESET:
                    __config_load(&sys_info);
                    config_deinit();
                    esp_restart();
                    break;

                case JOKE_MSG_CONFIG_RELOAD:
                    __config_load(&sys_info);
                    break;
            }
        }
    }
}

int joke_config_prepare(struct system_info *info)
{
    int ret = -123;

    msg_queue = xQueueCreate(2, sizeof(struct joke_msg));
    if (!msg_queue) {
        ESP_LOGE(TAG, "%s %d", __FILE__, __LINE__);
        goto error_out;
    }

    xTaskCreate(joke_msg_task, "msg",
                configMINIMAL_STACK_SIZE, msg_queue,
                configMAX_PRIORITIES-1, &msg_task);

    ret = config_init();
    if (ret) {
        ESP_LOGE(TAG, "%s %d %d", __FILE__, __LINE__, ret);
        goto error_out;
    }

    ret = esp_netif_init();
    if (ret) {
        ESP_LOGE(TAG, "%s %d %d", __FILE__, __LINE__, ret);
        goto error_out;
    }

    ret = esp_event_loop_create_default();
    if (ret) {
        ESP_LOGE(TAG, "%s %d %d", __FILE__, __LINE__, ret);
        goto error_out;
    }

    ret = nvs_flash_init();
    if (ret) {
        ESP_LOGE(TAG, "%s %d %d", __FILE__, __LINE__, ret);
        goto error_out;
    }

    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t wifi_config;

    ret = esp_wifi_init(&cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s %d %d", __FILE__, __LINE__, ret);
        goto error_out;
    }

    ret = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    if (ret) {
        ESP_LOGE(TAG, "%s %d %d", __FILE__, __LINE__, ret);
        goto error_out;
    }

    memset(&wifi_config, 0, sizeof(wifi_config_t));
    wifi_config.ap.max_connection = 1;
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    esp_read_mac(sys_info.mac, ESP_MAC_WIFI_SOFTAP);
    sprintf((char *)&wifi_config.ap.ssid, "Joke-%02X%02X", sys_info.mac[4], sys_info.mac[5]);

    ret = esp_wifi_set_mode(WIFI_MODE_AP);
    if (ret) {
        ESP_LOGE(TAG, "%s %d %d", __FILE__, __LINE__, ret);
        goto error_out;
    }

    ret = esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    if (ret) {
        ESP_LOGE(TAG, "%s %d %d", __FILE__, __LINE__, ret);
        goto error_out;
    }

    ret = esp_wifi_start();
    if (ret) {
        ESP_LOGE(TAG, "%s %d %d", __FILE__, __LINE__, ret);
        goto error_out;
    }

    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);

    char ip_addr[16];
    inet_ntoa_r(ip_info.ip.addr, ip_addr, 16);
    ESP_LOGI(TAG, "Set up softAP with IP: %s", ip_addr);

    dns_server_config_t config = DNS_SERVER_CONFIG_SINGLE("*" /* all A queries */, "WIFI_AP_DEF" /* softAP netif ID */);
    start_dns_server(&config);

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 10,
        .format_if_mount_failed = true
    };

    ret = esp_vfs_spiffs_register(&conf);
    if (ret) {
        ESP_LOGE(TAG, "%s %d %d", __FILE__, __LINE__, ret);
        goto error_out;
    }

    ret = http_server_start(msg_queue);
    if (ret)
        ESP_LOGE(TAG, "%s %d %d", __FILE__, __LINE__, ret);

error_out:
    return ret;
}

void joke_config_process(void)
{
    int count = 0;
    joke_config_prepare(&sys_info);
    sys_info.flags |= JOKE_FLAG_SETTING;
    dispart(sys_info.theme->config_wait, &sys_info);
    dis_wait_idle(DIS_WAIT_IDLE_MODE_LOOP);
    dis_setpowersave(&sys_info, 1);

    while(1)
    {
        vTaskDelay(10000/portTICK_PERIOD_MS);
        count++;
        if (count >= 12) {
            /* 2 mins, check any device are connected. */
            wifi_sta_list_t sta_list;
            esp_wifi_ap_get_sta_list(&sta_list);
            if (sta_list.num == 0) {
                ESP_LOGI(TAG, "Timeout, retting");
                esp_restart();
            }
        }
    }
}

void app_main(void)
{
    int reason;

    joke_core_prepare();
    /* Have to check fault state after joke_core_prepare called. */
    if (sys_info.flags & JOKE_FLAG_COREFAULT) {
        dispart(sys_info.theme->core_failure, &sys_info);
        joke_deep_sleep();
    }

    /* Check configure status and running mark. */
    if (!(sys_info.flags & JOKE_FLAG_CONFIGURED) ||
        !rtc_compat_check_running_mark()) {
        if (sys_info.wakeup_cause == JOKE_WAKEUP_BUTTON) {
            /* execute config. */
            joke_config_process();
        } else {
            dispart(sys_info.theme->welcome, &sys_info);
            rtc_compat_reset();
            joke_deep_sleep();
        }
    } else {
        if (sys_info.wakeup_cause == JOKE_WAKEUP_BUTTON) {
            /* execute config. */
            joke_config_process();
        } else {
            rtc_compat_get_wakeup_reason(&reason);
            rtc_compat_enable_update_intr();

            rtc_compat_get_datetime(&sys_info.dt);
            sys_info.dt.year += 2000;

            /* Read battery voltage once a day. */
            if ((sys_info.dt.day != sys_info.batt_mark) ||
                (sys_info.wakeup_cause & JOKE_WAKEUP_CHARGE)) {
                sys_info.batt_mark = sys_info.dt.day;
                read_battery_voltage(&sys_info);
                if (sys_info.batt_level == 0)
                    sys_info.flags |= JOKE_FLAG_BATTDEAD;
                else
                    sys_info.flags &= ~JOKE_FLAG_BATTDEAD;
            }

            if (sys_info.flags & JOKE_FLAG_BATTDEAD)
                dispart(sys_info.theme->battery_dead, &sys_info);
            else
                dispart(sys_info.theme->primary, &sys_info);
            joke_deep_sleep();
        }
    }
}
