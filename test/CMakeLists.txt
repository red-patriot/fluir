add_executable(fluir.compiler.test)
target_sources(
    fluir.compiler.test
    PRIVATE afg.test.cpp diagnostics.test.cpp parser.test.cpp
)

target_compile_definitions(
    fluir.compiler.test
    PRIVATE "TEST_FOLDER=std::filesystem::path(\"${CMAKE_CURRENT_SOURCE_DIR}\")"
)

target_link_libraries(
    fluir.compiler.test
    PRIVATE fluir::compiler GTest::gtest GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(fluir.compiler.test)
