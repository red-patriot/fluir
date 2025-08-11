if (PROJECT_SOURCE_DIR
    STREQUAL
    PROJECT_BINARY_DIR
)
    message(FATAL_ERROR "In-source builds are not allowed.\n"
                        "Build the project with `cmake -b build` instead.\n"
    )
endif ()
