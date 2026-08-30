find_package(GTest REQUIRED)

add_executable(fluir.editor.core.test smoke.test.cpp)

target_link_libraries(
    fluir.editor.core.test PRIVATE fluir::editor-core GTest::gtest_main
)

include(GoogleTest)
# Prefix the registered CTest names with the component so `ctest -R
# fluir.editor.core` selects them (the smoke test has no fixture-path parameter
# to match on, unlike the compiler suite).
gtest_discover_tests(fluir.editor.core.test TEST_PREFIX fluir.editor.core.)
