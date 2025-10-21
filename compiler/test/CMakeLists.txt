find_package(GTest REQUIRED)

add_executable(fluir.compiler.test)

set(FLUIR_UTILITY_TEST_SOURCES utility/diagnostics.test.cpp
                               utility/pass.test.cpp
)

set(FLUIR_FRONTEND_TEST_SOURCES
    detect_syntax_errors.test.cpp
    parser.test.cpp
    frontend/parse_number.test.cpp
)

set(FLUIR_ASG_TEST_SOURCES
    asg/asg_errors.test.cpp
    asg/asg_parser_integration.test.cpp
    asg/asg.test.cpp
)

set(FLUIR_BACKEND_TEST_SOURCES backend/bytecode_generator.test.cpp
                               backend/inspect_writer.test.cpp
)

set(FLUIR_TYPES_TEST_SOURCES types/type.test.cpp)

target_sources(
    fluir.compiler.test
    PRIVATE ${FLUIR_UTILITY_TEST_SOURCES}
            ${FLUIR_FRONTEND_TEST_SOURCES}
            ${FLUIR_ASG_TEST_SOURCES}
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
