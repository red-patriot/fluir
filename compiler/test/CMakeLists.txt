find_package(GTest REQUIRED)

add_executable(fluir.compiler.test)

set(FLUIR_UTILITY_TEST_SOURCES
    utility/diagnostic/sink.test.cpp
    utility/diagnostic/pretty_msg.test.cpp
    utility/topological_sort.test.cpp
)

set(FLUIR_FRONTEND_TEST_SOURCES
    detect_syntax_errors.test.cpp
    parser.test.cpp
    frontend/parse_number.test.cpp
    frontend/type_checker.test.cpp
    frontend/type_errors.test.cpp
    frontend/type_check_sanity.test.cpp
)

set(FLUIR_AST_TEST_SOURCES
    ast/ast_errors.test.cpp
    ast/ast_parser_integration.test.cpp
    ast/ast.test.cpp
)

set(FLUIR_BACKEND_TEST_SOURCES backend/bytecode_generator.test.cpp
                               backend/inspect_writer.test.cpp
)

set(FLUIR_TYPES_TEST_SOURCES
    types/type.test.cpp
    types/symbol_table.test.cpp
    types/overload_resolution.test.cpp
    types/builtin_symbols.test.cpp
)

target_sources(
    fluir.compiler.test
    PRIVATE ${FLUIR_UTILITY_TEST_SOURCES}
            ${FLUIR_FRONTEND_TEST_SOURCES}
            ${FLUIR_AST_TEST_SOURCES}
            ${FLUIR_BACKEND_TEST_SOURCES}
            ${FLUIR_TYPES_TEST_SOURCES}
            scope_guard.test.cpp
)
target_include_directories(
    fluir.compiler.test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(
    fluir.compiler.test
    PRIVATE "TEST_FOLDER=std::filesystem::path(\"${CMAKE_CURRENT_SOURCE_DIR}\")"
)
target_link_libraries(
    fluir.compiler.test
    PRIVATE fluir::compiler
            GTest::gtest
            GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(fluir.compiler.test)
