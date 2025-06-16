include(strict-warnings)

add_library(fluir.code STATIC)

add_library(
        fluir::code
        ALIAS
        fluir.code
)

target_sources(
        fluir.code
        PUBLIC FILE_SET
        CXX_MODULES
        FILES
        "modules/byte_code.ixx"
        "modules/code_chunk.ixx"
        "modules/instruction.ixx"
)

turn_up_warnings_on(fluir.code)

if (
        CMAKE_CXX_COMPILER_ID
        STREQUAL
        "MSVC"
)
    # This is necessary to resolve a warning about mismatched
    # compiler flags in modules in MSVC
    target_compile_options(fluir.code PUBLIC "/utf-8")
endif ()

target_include_directories(
        fluir.code PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
