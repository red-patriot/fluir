find_package(GTest REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(fluir.editor.test)

set(FLUIR_EDITOR_APP_TEST_SOURCES
    app/main_window.test.cpp
    app/module_page.test.cpp
    app/splash_widget.test.cpp
)

set(FLUIR_EDITOR_SCENE_TEST_SOURCES scene/graph_view.test.cpp)

set(FLUIR_EDITOR_CORE_TEST_SOURCES
    core/geometry.test.cpp
    core/identifier.test.cpp
    core/literal_text.test.cpp
    core/loader.test.cpp
    core/parse_tree_writer.test.cpp
)

set(FLUIR_EDITOR_VIEW_TEST_SOURCES
    view/draw/conditional.test.cpp
    view/draw/draw_utils.test.cpp
    view/draw/labels.test.cpp
    view/draw/size.test.cpp
    view/graph_geometry.test.cpp
)

target_sources(
    fluir.editor.test
    PRIVATE ${FLUIR_EDITOR_APP_TEST_SOURCES}
            ${FLUIR_EDITOR_CORE_TEST_SOURCES}
            ${FLUIR_EDITOR_SCENE_TEST_SOURCES}
            ${FLUIR_EDITOR_VIEW_TEST_SOURCES}
            main.cpp
            qt_platform.test.cpp
)

target_include_directories(
    fluir.editor.test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(
    fluir.editor.test
    PRIVATE TEST_FOLDER="${CMAKE_CURRENT_SOURCE_DIR}/programs"
)

target_link_libraries(
    fluir.editor.test
    PRIVATE fluir.libeditor
            Qt6::Test
            GTest::gtest
)

include(GoogleTest)
gtest_discover_tests(fluir.editor.test)
