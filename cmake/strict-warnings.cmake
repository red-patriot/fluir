function (turn_up_warnings_on target_name)
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
        message(WARNING "Clang is not yet fully supported, YMMV")
    elseif (
        CMAKE_CXX_COMPILER_ID
        STREQUAL
        "MSVC"
    )
        message(WARNING "MSVC is not yet fully supported, YMMV")
    else ()
        message(FATAL_ERROR "Must compile with GCC, Clang, or MSVC")
    endif ()
endfunction ()
