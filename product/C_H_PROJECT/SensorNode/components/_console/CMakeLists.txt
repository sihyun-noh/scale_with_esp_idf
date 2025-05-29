set(req console)

idf_component_register(SRCS "console_init.c"
                    INCLUDE_DIRS "."
                    REQUIRES ${req}
                    LDFRAGMENTS linker.lf
)
