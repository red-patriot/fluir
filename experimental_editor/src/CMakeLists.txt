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
    "core/annotate.cpp"
    "core/collecting_sink.cpp"
    "core/fields.cpp"
    "core/identifier.cpp"
    "core/intelligence.cpp"
    "core/literal_text.cpp"
    "core/node_access.cpp"
    "core/loader.cpp"
    "core/module_editor.cpp"
    "core/parse_tree_writer.cpp"
    "core/tree_edit.cpp"
    "core/tree_path.cpp"
    "core/viewport.cpp"
)

set(FLUIR_EDITOR_TRANSACTION_SOURCES
    "transaction/add_comment.cpp"
    "transaction/add_conduit.cpp"
    "transaction/add_decl.cpp"
    "transaction/add_node.cpp"
    "transaction/add_parameter.cpp"
    "transaction/add_port.cpp"
    "transaction/add_return.cpp"
    "transaction/delete.cpp"
    "transaction/delete_port.cpp"
    "transaction/edit_call_argument.cpp"
    "transaction/edit_call_node.cpp"
    "transaction/edit_comment.cpp"
    "transaction/edit_operator.cpp"
    "transaction/move.cpp"
    "transaction/move_port.cpp"
    "transaction/rename.cpp"
    "transaction/resize.cpp"
    "transaction/set_constant_value.cpp"
    "transaction/update_func_param.cpp"
)

# Q_OBJECT headers are listed so AUTOMOC finds them outside src/.
set(FLUIR_EDITOR_APP_SOURCES
    "app/main_window.cpp"
    "app/module_page.cpp"
    "app/module_document.cpp"
    "app/splash_widget.cpp"
    "../include/editor/app/main_window.hpp"
    "../include/editor/app/module_document.hpp"
    "../include/editor/app/module_page.hpp"
    "../include/editor/app/splash_widget.hpp"
)

set(FLUIR_EDITOR_SCENE_SOURCES
    "scene/components/branch_arrow.cpp"
    "scene/components/choice.cpp"
    "scene/components/frame.cpp"
    "scene/components/move_grip.cpp"
    "scene/components/port.cpp"
    "scene/components/resize_grip.cpp"
    "scene/components/scope_region.cpp"
    "scene/components/terminal.cpp"
    "scene/components/text_edit.cpp"
    "scene/components/toggle.cpp"
    "scene/elements/binary_element.cpp"
    "scene/elements/branch_element.cpp"
    "scene/elements/call_element.cpp"
    "scene/elements/comment_element.cpp"
    "scene/elements/conditional_element.cpp"
    "scene/elements/conduit_element.cpp"
    "scene/elements/constant_element.cpp"
    "scene/elements/element.cpp"
    "scene/elements/function_element.cpp"
    "scene/elements/rail_element.cpp"
    "scene/elements/unary_element.cpp"
    "scene/graph_scene.cpp"
    "scene/graph_view.cpp"
    "scene/item_style.cpp"
)

set(FLUIR_EDITOR_COMPONENT_SOURCES "components/menu.cpp"
                                   "components/text_field.cpp"
)

set(FLUIR_EDITOR_VIEW_SOURCES
    "view/graph_geometry.cpp"
    "view/graph_layout.cpp"
    "view/draw/binary.cpp"
    "view/draw/call.cpp"
    "view/draw/comment.cpp"
    "view/draw/conditional.cpp"
    "view/draw/constant.cpp"
    "view/draw/draw_utils.cpp"
    "view/draw/function.cpp"
    "view/draw/unary.cpp"
    "view/node_view.cpp"
)

set(FLUIR_EDITOR_TOOL_SOURCES
    "tools/bool_toggle_tool.cpp"
    "tools/branch_toggle_tool.cpp"
    "tools/choice_tool.cpp"
    "tools/completion_modal.cpp"
    "tools/completion_tool.cpp"
    "tools/conditional_header_menu.cpp"
    "tools/context_menu_tool.cpp"
    "tools/conduit_tool.cpp"
    "tools/drag_tool.cpp"
    "tools/function_header_menu.cpp"
    "tools/grid_steps.cpp"
    "tools/menu_popup.cpp"
    "tools/popup_tool.cpp"
    "tools/port_drag_tool.cpp"
    "tools/port_menu.cpp"
    "tools/rail_menu.cpp"
    "tools/select_tool.cpp"
    "tools/text_edit_tool.cpp"
    "tools/tool.cpp"
)

target_sources(
    fluir.libeditor
    PRIVATE ${FLUIR_EDITOR_CORE_SOURCES}
            ${FLUIR_EDITOR_COMPONENT_SOURCES}
            ${FLUIR_EDITOR_TRANSACTION_SOURCES}
            ${FLUIR_EDITOR_VIEW_SOURCES}
            ${FLUIR_EDITOR_TOOL_SOURCES}
            ${FLUIR_EDITOR_APP_SOURCES}
            ${FLUIR_EDITOR_SCENE_SOURCES}
)

add_subdirectory(assets/images)

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
