#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"

#include "jac.h"
#include "config.h"
#include "rx8025t.h"

extern RTC_DATA_ATTR struct jac_info info;
extern TaskHandle_t config_task;

static const char *TAG = "httpd";
static httpd_handle_t http_server = NULL;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
    } else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
    } else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "text/xml";
    }
    return httpd_resp_set_type(req, type);
}

#define GET_REQ_POWERSAVING         "/get/ps"
#define GET_REQ_VERSION             "/get/version"

static esp_err_t jac_get_req_handler(httpd_req_t *req)
{
    if (strcmp(req->uri, GET_REQ_POWERSAVING) == 0) {
        char buf[4];
        snprintf(buf, sizeof(buf), "%d", info.ps_begin);
        httpd_resp_sendstr_chunk(req, buf);
        httpd_resp_send_chunk(req, " ", 1);
        snprintf(buf, sizeof(buf), "%d", info.ps_end);
        httpd_resp_sendstr_chunk(req, buf);
        httpd_resp_sendstr_chunk(req, NULL);
        goto success;
    } else if (strcmp(req->uri, GET_REQ_VERSION) == 0) {
        const esp_partition_t *running = esp_ota_get_running_partition();
        esp_app_desc_t running_app_info;
        esp_ota_get_partition_description(running, &running_app_info);
        httpd_resp_sendstr_chunk(req, running_app_info.version);
        httpd_resp_sendstr_chunk(req, NULL);
        goto success;
    }

    return httpd_resp_send_404(req);
success:
    return ESP_OK;
}

#define SET_REQ_DT                  "/set/dt"
#define SET_REQ_NIGHT               "/set/ps"
#define SET_REQ_EXIT                "/set/exit"
#define SET_REQ_FIRSTBOOT           "/set/firstboot"

static esp_err_t jac_set_req_handler(httpd_req_t *req)
{
    if (strncmp(req->uri, SET_REQ_DT, strlen(SET_REQ_DT)) == 0) {
        /* set/dt?2024/4/13-6-16:56:39 */
        const char *conf = req->uri + strlen(SET_REQ_DT) + 1;
        sscanf(conf, "%hu/%hhu/%hhu-%hhu-%hhu:%hhu:%hhu",
                &info.dt.year, &info.dt.month,
                &info.dt.day, &info.dt.week,
                &info.dt.hours, &info.dt.minutes, &info.dt.seconds);
        xTaskGenericNotify(config_task, tskDEFAULT_INDEX_TO_NOTIFY, 0x01, eSetBits, NULL);
        goto success;
    } else if (strncmp(req->uri, SET_REQ_NIGHT, strlen(SET_REQ_NIGHT)) == 0) {
        /* set/ps?begin=xx&end=xx */
        const char *conf = req->uri + strlen(SET_REQ_NIGHT)+1;
        sscanf(conf, "begin=%hhu&end=%hhu", &info.ps_begin, &info.ps_end);
        xTaskGenericNotify(config_task, tskDEFAULT_INDEX_TO_NOTIFY, 0x02, eSetBits, NULL);
        goto success;
    } else if (strcmp(req->uri, SET_REQ_EXIT) == 0) {
        /* set/exit */
        xTaskGenericNotify(config_task, tskDEFAULT_INDEX_TO_NOTIFY, 0x80, eSetBits, NULL);
        goto success;
    } else if (strcmp(req->uri, SET_REQ_FIRSTBOOT) == 0) {
        /* set/firstboot */
        xTaskGenericNotify(config_task, tskDEFAULT_INDEX_TO_NOTIFY, 0x04, eSetBits, NULL);
        goto success;
    }

    return httpd_resp_send_404(req);
success:
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t http_get_handler(httpd_req_t *req)
{
    char buf[512];
    int i;
    FILE *fp;
    esp_err_t res;

    ESP_LOGI(TAG, "uri: %s", req->uri);

    if (strncmp(req->uri, "/get/", 5) == 0)
        return jac_get_req_handler(req);
    if (strncmp(req->uri, "/set/", 5) == 0)
        return jac_set_req_handler(req);

    strcpy(buf, "/spiffs");
    if (strlen(req->uri) == 1 && strcmp(req->uri, "/") == 0)
        strcat(buf, "/index.html");
    else
        strcat(buf, req->uri);

    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    set_content_type_from_file(req, buf);

    strcat(buf, ".gz");
    fp = fopen(buf, "r");
    if (fp == NULL)
        return httpd_resp_send_404(req);

    do {
        i = fread(buf, 1, 512, fp);
        res = httpd_resp_send_chunk(req, buf, i);
    } while (i == 512 && res == ESP_OK);

    httpd_resp_send_chunk(req, NULL, 0);
    fclose(fp);

    return ESP_OK;
}

enum {
    UG_WAIT_LABEL = 0,
    UG_WRITE_RES,
    UG_WRITE_WEB,
    UG_WRITE_APP,
};

static esp_err_t http_post_handler(httpd_req_t *req)
{
    uint8_t upgrade_state = 0;
    char buf[512];
    uint32_t remaining = req->content_len;
    int ret, recv_len = 0, expect_len = 0;
    uint32_t fid = 0, flen = 0, write_offset = 0;
    const esp_partition_t *partition = NULL;
    esp_ota_handle_t update_handle = 0 ;

    ESP_LOGI("upgrade", "content length %d", req->content_len);
    while (remaining > 0) {
        ret = httpd_req_recv(req, expect_len > 0 ? buf + 512 - expect_len : buf, expect_len > 0 ? expect_len : 512);
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            continue;
        } else if (ret <= 0) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Protocol Error");
            return ESP_FAIL;
        }

        recv_len += ret;
        if (recv_len < 512) {
            expect_len = 512 - recv_len;
            continue;
        }

        switch (upgrade_state) {
            case UG_WAIT_LABEL: {
                fid = *((uint32_t *)&buf[0]);
                flen = *((uint32_t *)&buf[4]);
                ESP_LOGI("upgrade", "fid %ld", fid);
                switch (fid) {
                    case 1:
                        upgrade_state = UG_WRITE_RES;
                        partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, 0x40, "res");
                        write_offset = 0;
                        esp_partition_erase_range(partition, 0, flen);
                        break;

                    case 2:
                        upgrade_state = UG_WRITE_WEB;
                        partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, 0x82, "storage");
                        write_offset = 0;
                        esp_partition_erase_range(partition, 0, flen);
                        break;

                    case 3:
                        upgrade_state = UG_WRITE_APP;
                        partition = esp_ota_get_next_update_partition(NULL);
                        esp_ota_begin(partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
                        break;

                    default:
                        break;
                }
                break;
            }

            case UG_WRITE_RES:
            case UG_WRITE_WEB: {
                esp_partition_write(partition, write_offset, buf, recv_len);
                write_offset += recv_len;
                flen -= recv_len;
                if (flen == 0) {
                    upgrade_state = UG_WAIT_LABEL;
                    ESP_LOGI("upgrade", "fid %ld done", fid);
                }
                break;
            }

            case UG_WRITE_APP: {
                if (esp_ota_write(update_handle, (const void *)buf, recv_len) != ESP_OK) {
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Flash Error");
                    return ESP_FAIL;
                }
                break;
            }
        }

        remaining -= recv_len;
        recv_len = 0;
        expect_len = 0;
    }

    esp_ota_end(update_handle);
    /* App partition data will be the last partition in firmware. */
    esp_ota_set_boot_partition(partition);

    httpd_resp_sendstr(req, "Firmware update complete, rebooting now!\n");
    vTaskDelay(500 / portTICK_PERIOD_MS);
    esp_restart();

    return ESP_OK;
}

static const httpd_uri_t http_get = {
    .uri      = "/*",
    .method   = HTTP_GET,
    .handler  = http_get_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t http_post = {
    .uri      = "/*",
    .method   = HTTP_POST,
    .handler  = http_post_handler,
    .user_ctx = NULL,
};

int http_server_start(void)
{
    esp_err_t res;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.uri_match_fn = httpd_uri_match_wildcard;
    res = httpd_start(&http_server, &config);
    if (res != ESP_OK)
        goto out;

    res = httpd_register_uri_handler(http_server, &http_get);
    res = httpd_register_uri_handler(http_server, &http_post);
    ESP_LOGI(TAG, "server started");

out:
    return res;
}

void http_server_stop(void)
{
    httpd_stop(http_server);
    ESP_LOGI(TAG, "server stoped");
}
