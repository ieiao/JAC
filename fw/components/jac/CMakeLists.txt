idf_component_register(
    SRC_DIRS drivers u8g2/csrc simple_theme http_server
    INCLUDE_DIRS . drivers u8g2/csrc simple_theme http_server
    REQUIRES driver esp_partition esp_adc nvs_flash esp_netif esp_http_server app_update
)
