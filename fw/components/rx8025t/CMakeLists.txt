set(rx8025t_srcs
    "rx8025t.c"
)

idf_component_register(
    SRCS ${rx8025t_srcs}
    INCLUDE_DIRS "."
    REQUIRES compat
)
