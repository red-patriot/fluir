find_package(GTest REQUIRED)

add_executable(fluir.editor.test)

set(FLUIR_EDITOR_COMPONENTS_TEST_SOURCES components/menu.test.cpp
                                         components/text_field.test.cpp
)

set(FLUIR_EDITOR_CORE_TEST_SOURCES
    core/fields.test.cpp
    core/geometry.test.cpp
    core/identifier.test.cpp
    core/intelligence.test.cpp
    core/literal_text.test.cpp
    core/node_access.test.cpp
    core/loader.test.cpp
    core/module_editor.test.cpp
    core/parse_tree_writer.test.cpp
    core/tree_edit.test.cpp
    core/tree_path.test.cpp
    core/viewport.test.cpp
)

set(FLUIR_EDITOR_PAGES_TEST_SOURCES pages/module.test.cpp pages/splash.test.cpp)

set(FLUIR_EDITOR_TOOLS_TEST_SOURCES
    tools/bool_toggle_tool.test.cpp
    tools/completion_modal.test.cpp
    tools/completion_tool.test.cpp
    tools/conduit_tool.test.cpp
    tools/context_menu_tool.test.cpp
    tools/drag_tool.test.cpp
    tools/function_header_menu.test.cpp
    tools/menu_popup.test.cpp
    tools/operator_tool.test.cpp
    tools/pan_zoom_tool.test.cpp
    tools/popup_tool.test.cpp
    tools/rail_menu.test.cpp
    tools/select_tool.test.cpp
    tools/text_edit_tool.test.cpp
    tools/tool_chain.test.cpp
    tools/type_tool.test.cpp
)

set(FLUIR_EDITOR_TRANSACTION_TEST_SOURCES transaction/transaction.test.cpp)

set(FLUIR_EDITOR_VIEW_TEST_SOURCES
    view/draw/conditional.test.cpp
    view/draw/draw_utils.test.cpp
    view/draw/labels.test.cpp
    view/graph_geometry.test.cpp
    view/graph_draw.test.cpp
    view/graph_layout.test.cpp
    view/toolbar.test.cpp
)

target_sources(
    fluir.editor.test
    PRIVATE ${FLUIR_EDITOR_COMPONENTS_TEST_SOURCES}
            ${FLUIR_EDITOR_CORE_TEST_SOURCES}
            ${FLUIR_EDITOR_PAGES_TEST_SOURCES}
            ${FLUIR_EDITOR_TOOLS_TEST_SOURCES}
            ${FLUIR_EDITOR_TRANSACTION_TEST_SOURCES}
            ${FLUIR_EDITOR_VIEW_TEST_SOURCES}
            input.test.cpp
)

target_include_directories(
    fluir.editor.test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
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
