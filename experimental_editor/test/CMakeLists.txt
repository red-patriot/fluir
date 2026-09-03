find_package(GTest REQUIRED)

add_executable(
    fluir.editor.test
    editor_context.test.cpp
    input.test.cpp
    loader.test.cpp
    viewport.test.cpp
    render_graph.test.cpp
)

target_compile_definitions(
    fluir.editor.test
    PRIVATE TEST_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/programs"
)

target_link_libraries(
    fluir.editor.test PRIVATE fluir.libeditor GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(fluir.editor.test)
