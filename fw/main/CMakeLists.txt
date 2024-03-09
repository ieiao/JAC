set(srcs
    "joke.c"
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "."
    PRIV_REQUIRES compat simple_theme esp_adc dns_server esp_wifi nvs_flash spiffs http_server
)

spiffs_create_partition_image(storage ../spiffs_image)
