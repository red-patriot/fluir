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

target_sources(
    fluir.compiler.test
    PRIVATE ${FLUIR_UTILITY_TEST_SOURCES}
            asg.test.cpp
            detect_syntax_errors.test.cpp
            parser.test.cpp
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
