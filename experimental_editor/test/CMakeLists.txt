find_package(GTest REQUIRED)

add_executable(
    fluir.editor.test
    identifier.test.cpp
    intelligence.test.cpp
    input.test.cpp
    literal_text.test.cpp
    loader.test.cpp
    menu.test.cpp
    menu_popup.test.cpp
    parse_tree_writer.test.cpp
    viewport.test.cpp
    graph_geometry.test.cpp
    graph_layout.test.cpp
    graph_draw.test.cpp
    draw_utils.test.cpp
    text_field.test.cpp
    toolbar.test.cpp
    tool_chain.test.cpp
    pan_zoom_tool.test.cpp
    popup_tool.test.cpp
    completion_modal.test.cpp
    completion_tool.test.cpp
    context_menu_tool.test.cpp
    function_header_menu.test.cpp
    rail_menu.test.cpp
    operator_tool.test.cpp
    type_tool.test.cpp
    select_tool.test.cpp
    drag_tool.test.cpp
    conduit_tool.test.cpp
    bool_toggle_tool.test.cpp
    text_edit_tool.test.cpp
    module.test.cpp
    splash.test.cpp
    module_editor.test.cpp
    tree_edit.test.cpp
    tree_path.test.cpp
    transaction.test.cpp
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
