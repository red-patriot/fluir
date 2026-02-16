find_package(
    fmt
    CONFIG
    REQUIRED
)
find_package(tinyxml2 REQUIRED)

include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    REV
    v3.2
)
FetchContent_MakeAvailable(argparse)

include(strict-warnings)

add_library(fluir.libcompiler)
add_library(
    fluir::compiler
    ALIAS
    fluir.libcompiler
)

set(FLUIR_COMPILER_FRONTEND_SOURCES
    "frontend/parser.cpp"
    "frontend/ast_builder.cpp"
    "frontend/text_parse.cpp"
    "frontend/type_checker.cpp"
)

set(FLUIR_COMPILER_BACKEND_SOURCES
    "backend/bytecode_generator.cpp"
    "backend/code_writer.cpp"
    "backend/inspect_writer.cpp"
)

set(FLUIR_COMPILER_DEBUG_SOURCES "debug/ast_printer.cpp"
                                 "debug/parse_tree_printer.cpp"
)

set(FLUIR_COMPILER_TYPES_SOURCES
    "types/builtin_symbols.cpp"
    "types/conversion.cpp"
    "types/operator_definition.cpp"
    "types/symbol_table.cpp"
    "types/type.cpp"
)

set(FLUIR_COMPILER_UTILITY_SOURCES
    "utility/diagnostic/pretty_msg.cpp"
    "utility/diagnostic/ostream_sink.cpp"
    "utility/diagnostic/colored_stdout_sink.cpp"
    "utility/options.cpp"
)

target_sources(
    fluir.libcompiler
    PRIVATE ${FLUIR_COMPILER_FRONTEND_SOURCES}
            ${FLUIR_COMPILER_BACKEND_SOURCES}
            ${FLUIR_COMPILER_DEBUG_SOURCES}
            ${FLUIR_COMPILER_TYPES_SOURCES}
            ${FLUIR_COMPILER_UTILITY_SOURCES}
)

turn_up_warnings_on(fluir.libcompiler)

target_include_directories(
    fluir.libcompiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(
    fluir.libcompiler
    PUBLIC fluir::code
           fluir::utility
           fmt::fmt
           tinyxml2::tinyxml2
           argparse::argparse
)

add_executable(fluir.compiler "main.cpp")

target_link_libraries(fluir.compiler PUBLIC fluir.libcompiler)
