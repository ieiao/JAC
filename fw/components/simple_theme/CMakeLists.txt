set(simple_theme_srcs
    "simple_theme.c"
    "theme_default.c"
)

idf_component_register(
    SRCS ${simple_theme_srcs}
    INCLUDE_DIRS "."
    REQUIRES compat u8g2
)
