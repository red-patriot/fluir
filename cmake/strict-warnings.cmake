function(turn_up_warnings_on target_name)
    message("TARGET: ${target_name}")

    if (CMAKE_CXX_COMPILER_ID
            STREQUAL
            "GNU"
    )
        target_compile_options(
                ${target_name}
                PRIVATE -Wall
                -Wextra
                -Werror
                -pedantic
        )
    elseif (
            CMAKE_CXX_COMPILER_ID
            STREQUAL
            "Clang"
    )
        target_compile_options(
                ${target_name}
                PRIVATE -Wall
                -Wextra
                -Wshadow
                -Wcast-align
                -Wunused
                -Wunreachable-code
                -Wformat=2
                -Woverloaded-virtual
                -Wsign-promo
                -Werror
                -pedantic
        )
    elseif (
            CMAKE_CXX_COMPILER_ID
            STREQUAL
            "MSVC"
    )
        target_compile_options(${target_name}
                PRIVATE /W4 /WX /permissive-)
    else ()
        message(FATAL_ERROR "Must compile with GCC, Clang, or MSVC")
    endif ()
endfunction()
