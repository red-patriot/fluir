add_executable(fluir.compiler.test)
target_sources(fluir.compiler.test PRIVATE dummy.test.cpp)
target_link_libraries(
    fluir.compiler.test
    PRIVATE fluir::compiler GTest::gtest GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(fluir.compiler.test)
