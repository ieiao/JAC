set(srcs
    "main.c"
)

idf_component_register(
    SRCS ${srcs}
    PRIV_REQUIRES jac spiffs
)

spiffs_create_partition_image(storage ../spiffs_image)
# spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)
