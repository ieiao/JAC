#include "nvs_flash.h"
#include "nvs.h"
#include "system_info.h"
#include "config_compat.h"

static nvs_handle_t handle;

int config_init(void)
{
    int ret;

    ret = nvs_flash_init();
    if (ret)
        goto out;

    ret = nvs_open("joke", NVS_READWRITE, &handle);
out:
    return ret;
}

void config_deinit(void)
{
    if (handle) {
        nvs_commit(handle);
        nvs_close(handle);
        handle = 0;
    }
}

int config_erase(void)
{
    int ret = 0;

    if (handle) {
        nvs_erase_all(handle);
    }

    return ret;
}

int __config_load(struct system_info *info)
{
    int ret;
    uint8_t out_value;

    ret = nvs_get_u8(handle, KEY_CONFIGURED, &out_value);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        /* This error means this is the first time boot,
         * nothing needs to do, so just return. */
        ret = 0;
        goto out;
    } else if (ret != ESP_OK) {
        goto out;
    }

    if (out_value)
        info->flags |= JOKE_FLAG_CONFIGURED;

    ret = nvs_get_u8(handle, KEY_PS_BEGIN, &out_value);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        /* Not set this function, just return. */
        ret = 0;
        goto out;
    } else if (ret != ESP_OK) {
        goto out;
    }

    info->ps_begin = out_value;

    ret = nvs_get_u8(handle, KEY_PS_END, &out_value);
    if (ret != ESP_OK) {
        /* PS_BEGIN and PS_END will setting by same time. */
        goto out;
    }

    info->ps_end = out_value;

out:
    return ret;
}

int config_load(struct system_info *info)
{
    int ret;

    ret = config_init();
    if (ret)
        goto out;

    ret = __config_load(info);

out:
    config_deinit();
    return ret;
}

int config_set(const char *k, uint8_t v)
{
    if (!handle)
        return -1;

    return nvs_set_u8(handle, k, v);
}
