#include "esp_partition.h"

int mmap_data_partition(esp_partition_subtype_t subtype, const char* label,
                        const void **ptr, esp_partition_mmap_handle_t *handle)
{
    esp_partition_iterator_t iterator;
    const esp_partition_t *partition;
    int ret;

    iterator = esp_partition_find(ESP_PARTITION_TYPE_DATA, subtype, label);
    if (!iterator) {
        ret = -1;
        goto out;
    }

    partition = esp_partition_get(iterator);
    ret = esp_partition_mmap(partition, 0, partition->size, ESP_PARTITION_MMAP_DATA, ptr, handle);
out:
    return ret;
}

void munmap_data_partition(esp_partition_mmap_handle_t handle)
{
    esp_partition_munmap(handle);
}

const uint8_t *u8g2_font_logisoso62_tn;
const uint8_t *u8g2_font_logisoso28_tn;
const uint8_t *u8g2_font_wqy16_t_gb2312a;
const uint8_t *u8g2_font_siji_t_6x10;

#define OFFSET_U8G2_FONT_LOGISOSO62_TN 0
#define OFFSET_U8G2_FONT_LOGISOSO28_TN 982
#define OFFSET_U8G2_FONT_WQY16_T_GB2312A 1411
#define OFFSET_U8G2_FONT_SIJI_T_6X10 165379

const uint8_t *res_tree_xbm;
const uint8_t *res_batt_dead;
const uint8_t *res_powersaving_p1;
const uint8_t *res_powersaving_p2;
const uint8_t *res_powersaving_p3;

#define OFFSET_BATT_DEAD_XBM_BIN 0
#define OFFSET_MOON_XPMO0_BIN 420
#define OFFSET_MOON_XPMO1_BIN 5156
#define OFFSET_MOON_XPMO2_BIN 9892
#define OFFSET_TREE_XBM_BIN 14628

esp_partition_mmap_handle_t fonts_partition_handle;
esp_partition_mmap_handle_t res_partition_handle;

int resources_prepare(void)
{
    int ret;
    const void *ptr;

    ret = mmap_data_partition(0x40, "fonts", &ptr, &fonts_partition_handle);
    if (ret)
        goto out;

    u8g2_font_logisoso62_tn = ptr + OFFSET_U8G2_FONT_LOGISOSO62_TN;
    u8g2_font_logisoso28_tn = ptr + OFFSET_U8G2_FONT_LOGISOSO28_TN;
    u8g2_font_wqy16_t_gb2312a = ptr + OFFSET_U8G2_FONT_WQY16_T_GB2312A;
    u8g2_font_siji_t_6x10 = ptr + OFFSET_U8G2_FONT_SIJI_T_6X10;

    ret = mmap_data_partition(0x40, "res", &ptr, &res_partition_handle);
    if (ret)
        goto out;

    res_tree_xbm = ptr + OFFSET_TREE_XBM_BIN;
    res_batt_dead = ptr + OFFSET_BATT_DEAD_XBM_BIN;
    res_powersaving_p1 = ptr + OFFSET_MOON_XPMO0_BIN;
    res_powersaving_p2 = ptr + OFFSET_MOON_XPMO1_BIN;
    res_powersaving_p3 = ptr + OFFSET_MOON_XPMO2_BIN;

out:
    return ret;
}
