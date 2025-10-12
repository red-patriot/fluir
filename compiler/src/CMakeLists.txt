find_package(
    fmt
    CONFIG
    REQUIRED
)
find_package(tinyxml2 REQUIRED)

include(strict-warnings)

add_library(fluir.libcompiler)
add_library(
    fluir::compiler
    ALIAS
    fluir.libcompiler
)

set(FLUIR_COMPILER_FRONTEND_SOURCES
    "frontend/parser.cpp"
    "frontend/asg_builder.cpp"
    "frontend/text_parse.cpp"
)

set(FLUIR_COMPILER_BACKEND_SOURCES
    "backend/bytecode_generator.cpp"
    "backend/code_writer.cpp"
    "backend/inspect_writer.cpp"
)

set(FLUIR_COMPILER_DEBUG_SOURCES "debug/asg_printer.cpp"
                                 "debug/parse_tree_printer.cpp"
)

target_sources(
    fluir.libcompiler
    PRIVATE ${FLUIR_COMPILER_FRONTEND_SOURCES}
            ${FLUIR_COMPILER_BACKEND_SOURCES}
            ${FLUIR_COMPILER_DEBUG_SOURCES}
            "utility/diagnostics.cpp"
)

turn_up_warnings_on(fluir.libcompiler)

target_include_directories(
    fluir.libcompiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(
    fluir.libcompiler
    PUBLIC fluir::code
           fmt::fmt
           tinyxml2::tinyxml2
)

add_executable(fluir.compiler "main.cpp")

target_link_libraries(fluir.compiler PRIVATE fluir::compiler)
