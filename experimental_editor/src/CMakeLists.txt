find_package(
    fmt
    CONFIG
    REQUIRED
)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)

include(strict-warnings)

add_library(fluir.libeditor)

set_target_properties(fluir.libeditor PROPERTIES AUTOMOC ON)

target_include_directories(
    fluir.libeditor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

set(FLUIR_EDITOR_CORE_SOURCES
    "core/collecting_sink.cpp"
    "core/identifier.cpp"
    "core/literal_text.cpp"
    "core/loader.cpp"
    "core/parse_tree_writer.cpp"
)

# Q_OBJECT headers are listed so AUTOMOC finds them outside src/.
set(FLUIR_EDITOR_APP_SOURCES
    "app/main_window.cpp"
    "app/module_page.cpp"
    "app/splash_widget.cpp"
    "../include/editor/app/main_window.hpp"
    "../include/editor/app/module_page.hpp"
    "../include/editor/app/splash_widget.hpp"
)

set(FLUIR_EDITOR_SCENE_SOURCES "scene/graph_view.cpp" "scene/item_style.cpp")

set(FLUIR_EDITOR_VIEW_SOURCES
    "view/graph_geometry.cpp"
    "view/draw/binary.cpp"
    "view/draw/call.cpp"
    "view/draw/comment.cpp"
    "view/draw/conditional.cpp"
    "view/draw/constant.cpp"
    "view/draw/draw_utils.cpp"
    "view/draw/function.cpp"
    "view/draw/unary.cpp"
)

target_sources(
    fluir.libeditor
    PRIVATE ${FLUIR_EDITOR_CORE_SOURCES}
            ${FLUIR_EDITOR_VIEW_SOURCES}
            ${FLUIR_EDITOR_APP_SOURCES}
            ${FLUIR_EDITOR_SCENE_SOURCES}
)

# Keeps these resources linked through the static library without
# Q_INIT_RESOURCE.
qt_add_resources(
    fluir.libeditor
    "editor_assets"
    PREFIX
    "/"
    BASE
    "assets"
    FILES
    "assets/fonts/DejaVuSansMono.ttf"
    "assets/images/branchArrow.svg"
    "assets/images/dragHandle.svg"
    "assets/images/false.svg"
    "assets/images/true.svg"
    "assets/images/XYResizeHandle.svg"
)

target_link_libraries(
    fluir.libeditor
    PUBLIC fmt::fmt
           Qt6::Widgets
           Qt6::Svg
           fluir::compiler
           fluir::utility
)

add_executable(fluir.editor "main.cpp")

target_link_libraries(fluir.editor PUBLIC fluir.libeditor)

turn_up_warnings_on(fluir.editor)
