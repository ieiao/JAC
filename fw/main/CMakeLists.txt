set(srcs
    "jac.c"
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "."
    PRIV_REQUIRES jac esp_adc esp_wifi nvs_flash spiffs
)

spiffs_create_partition_image(storage ../spiffs_image)
# spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)
