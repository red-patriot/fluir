find_package(GTest REQUIRED)

add_executable(
    fluir.editor.test
    input.test.cpp
    interaction.test.cpp
    layer.test.cpp
    loader.test.cpp
    parse_tree_writer.test.cpp
    viewport.test.cpp
    graph_draw.test.cpp
    graph_geometry.test.cpp
    actor.test.cpp
    node_drag.test.cpp
    function_decl_drag.test.cpp
    button_actor.test.cpp
    scene.test.cpp
    module.test.cpp
    header_bar.test.cpp
    splash.test.cpp
    selection.test.cpp
    tree_edit.test.cpp
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
