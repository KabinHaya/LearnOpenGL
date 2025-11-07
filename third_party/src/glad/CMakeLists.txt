add_library(glad
    STATIC
        glad.c
)

target_include_directories(glad
    PUBLIC
        ${MYLIB_INCLUDE_DIR}
)