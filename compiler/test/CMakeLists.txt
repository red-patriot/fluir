include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt
    ON
    CACHE BOOL
          ""
          FORCE
)
FetchContent_MakeAvailable(googletest)

add_executable(fluir.compiler.test)

set(FLUIR_UTILITY_TEST_SOURCES utility/diagnostics.test.cpp
                               utility/results.test.cpp
)

set(FLUIR_ASG_TEST_SOURCES
    asg/asg_errors.test.cpp
    asg/asg_parser_integration.test.cpp
    asg/asg.test.cpp
)

set(FLUIR_BACKEND_TEST_SOURCES backend/bytecode_generator.test.cpp
                               backend/inspect_writer.test.cpp
)

target_sources(
    fluir.compiler.test
    PRIVATE ${FLUIR_UTILITY_TEST_SOURCES}
            ${FLUIR_ASG_TEST_SOURCES}
            ${FLUIR_BACKEND_TEST_SOURCES}
            detect_syntax_errors.test.cpp
            parser.test.cpp
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
