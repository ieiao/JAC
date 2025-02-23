#include "esp_partition.h"

int mmap_data_partition(esp_partition_subtype_t subtype, const char* label,
                        const void **ptr, esp_partition_mmap_handle_t *handle)
{
    const esp_partition_t *partition;

    partition = esp_partition_get(esp_partition_find(ESP_PARTITION_TYPE_DATA, subtype, label));
    esp_partition_iterator_release(NULL);
    return esp_partition_mmap(partition, 0, partition->size, ESP_PARTITION_MMAP_DATA, ptr, handle);
}

void munmap_data_partition(esp_partition_mmap_handle_t handle)
{
    esp_partition_munmap(handle);
}

const uint8_t *u8g2_font_logisoso62_tn;
const uint8_t *u8g2_font_logisoso28_tn;
const uint8_t *u8g2_font_wqy16_t_gb2312a;
const uint8_t *u8g2_font_siji_t_6x10;
const uint8_t *u8g2_font_luIS12;
const uint8_t *u8g2_font_luIS24;
const uint8_t *u8g2_font_luRS10;
const uint8_t *u8g2_font_courR10;
const uint8_t *u8g2_font_helvR10;
const uint8_t *xbm_tree;
const uint8_t *xbm_batt_dead;
const uint8_t *xbm_jac_p1;
const uint8_t *xbm_jac_p2;
const uint8_t *xbm_jac_p3;

#define OFFSET_U8G2_FONT_LOGISOSO62_TN 0
#define OFFSET_U8G2_FONT_LOGISOSO28_TN 982
#define OFFSET_U8G2_FONT_WQY16_T_GB2312A 1411
#define OFFSET_U8G2_FONT_SIJI_T_6X10 165379
#define OFFSET_U8G2_FONT_LUIS12_TR 175908
#define OFFSET_U8G2_FONT_LUIS24_TR 177758
#define OFFSET_U8G2_FONT_LURS10_TR 181769
#define OFFSET_U8G2_FONT_COURR10_TR 183064
#define OFFSET_U8G2_FONT_HELVR10_TR 184368
#define OFFSET_BATT_DEAD_XBM_BIN 185657
#define OFFSET_MOON_XPMO0_BIN 186112
#define OFFSET_MOON_XPMO1_BIN 190016
#define OFFSET_MOON_XPMO2_BIN 193920
#define OFFSET_TREE_XBM_BIN 197824

esp_partition_mmap_handle_t res_partition_handle;

int resources_init(void)
{
    int ret;
    const void *ptr;

    ret = mmap_data_partition(0x40, "res", &ptr, &res_partition_handle);
    if (ret)
        goto out;

    u8g2_font_logisoso62_tn   = ptr + OFFSET_U8G2_FONT_LOGISOSO62_TN;
    u8g2_font_logisoso28_tn   = ptr + OFFSET_U8G2_FONT_LOGISOSO28_TN;
    u8g2_font_wqy16_t_gb2312a = ptr + OFFSET_U8G2_FONT_WQY16_T_GB2312A;
    u8g2_font_siji_t_6x10     = ptr + OFFSET_U8G2_FONT_SIJI_T_6X10;
    u8g2_font_luIS12          = ptr + OFFSET_U8G2_FONT_LUIS12_TR;
    u8g2_font_luIS24          = ptr + OFFSET_U8G2_FONT_LUIS24_TR;
    u8g2_font_luRS10          = ptr + OFFSET_U8G2_FONT_LURS10_TR;
    u8g2_font_courR10         = ptr + OFFSET_U8G2_FONT_COURR10_TR;
    u8g2_font_helvR10         = ptr + OFFSET_U8G2_FONT_HELVR10_TR;
    xbm_tree                  = ptr + OFFSET_TREE_XBM_BIN;
    xbm_batt_dead             = ptr + OFFSET_BATT_DEAD_XBM_BIN;
    xbm_jac_p1                = ptr + OFFSET_MOON_XPMO0_BIN;
    xbm_jac_p2                = ptr + OFFSET_MOON_XPMO1_BIN;
    xbm_jac_p3                = ptr + OFFSET_MOON_XPMO2_BIN;

out:
    return ret;
}

void resources_deinit(void)
{
    munmap_data_partition(res_partition_handle);
}
