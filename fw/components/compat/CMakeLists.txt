set(adapter_srcs
    "adc_compat.c"
    "config_compat.c"
    "dis_compat.c"
    "i2c_compat.c"
    "io_compat.c"
    "rtc_compat.c"
    "resources.c"
)

idf_component_register(
    SRCS ${adapter_srcs}
    INCLUDE_DIRS "."
    REQUIRES driver u8g2 rx8025t esp_partition esp_adc nvs_flash
)
