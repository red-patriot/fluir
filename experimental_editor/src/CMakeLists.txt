find_package(
    fmt
    CONFIG
    REQUIRED
)

include(FetchContent)
set(SDL_SHARED
    OFF
    CACHE BOOL
          ""
          FORCE
)
set(SDL_STATIC
    ON
    CACHE BOOL
          ""
          FORCE
)
set(SDL_TEST
    OFF
    CACHE BOOL
          ""
          FORCE
)
set(SDL_EXAMPLES
    OFF
    CACHE BOOL
          ""
          FORCE
)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.24
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3)

# SDL3's headers are third-party: consume them as system includes so our own
# -Werror -pedantic warnings don't fire on them.
if (TARGET SDL3-static)
    set_target_properties(SDL3-static PROPERTIES SYSTEM TRUE)
endif ()

set(NFD_BUILD_TESTS
    OFF
    CACHE BOOL
          ""
          FORCE
)
FetchContent_Declare(
    nfd
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
    GIT_TAG v1.2.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nfd)

# NFD's headers are third-party: consume them as system includes so our own
# -Werror -pedantic warnings don't fire on them.
if (TARGET nfd)
    set_target_properties(nfd PROPERTIES SYSTEM TRUE)
endif ()

set(SDLTTF_VENDORED
    ON
    CACHE BOOL
          ""
          FORCE
)
foreach (
    opt
    SDLTTF_HARFBUZZ
    SDLTTF_PLUTOSVG
    SDLTTF_SAMPLES
    SDLTTF_INSTALL
)
    set(${opt}
        OFF
        CACHE BOOL
              ""
              FORCE
    )
endforeach ()
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL
          ""
          FORCE
)
FetchContent_Declare(
    SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-3.2.2
    GIT_SHALLOW TRUE
    GIT_SUBMODULES "external/freetype"
)
FetchContent_MakeAvailable(SDL3_ttf)

# SDL_ttf's headers are third-party: consume them as system includes so our own
# -Werror -pedantic warnings don't fire on them.
if (TARGET SDL3_ttf-static)
    set_target_properties(SDL3_ttf-static PROPERTIES SYSTEM TRUE)
endif ()

include(strict-warnings)
include(embed-file)

add_library(fluir.libeditor)

target_include_directories(
    fluir.libeditor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

set(FLUIR_EDITOR_CORE_SOURCES
    "core/collecting_sink.cpp"
    "core/graph_geometry.cpp"
    "core/identifier.cpp"
    "core/literal_text.cpp"
    "core/loader.cpp"
    "core/module_editor.cpp"
    "core/parse_tree_writer.cpp"
    "core/tree_edit.cpp"
    "core/tree_path.cpp"
    "core/viewport.cpp"
)

set(FLUIR_EDITOR_TRANSACTION_SOURCES
    "transaction/delete.cpp"
    "transaction/edit_call_argument.cpp"
    "transaction/edit_call_node.cpp"
    "transaction/move.cpp"
    "transaction/rename.cpp"
    "transaction/resize.cpp"
    "transaction/set_constant_value.cpp"
    "transaction/update_func_param.cpp"
)

set(FLUIR_EDITOR_PAGE_SOURCES
    "pages/module.cpp"
    "pages/page.cpp"
    "pages/splash.cpp"
)

set(FLUIR_EDITOR_COMPONENT_SOURCES "components/text_field.cpp")

set(FLUIR_EDITOR_VIEW_SOURCES
    "view/graph_draw.cpp"
    "view/graph_layout.cpp"
    "view/node_view.cpp"
    "view/toolbar.cpp"
)

set(FLUIR_EDITOR_TOOL_SOURCES
    "tools/drag_tool.cpp"
    "tools/pan_zoom_tool.cpp"
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
            ${FLUIR_EDITOR_PAGE_SOURCES}
            "sdl_renderer.cpp"
            "input.cpp"
            "app.cpp"
)

fluir_embed_file(
    fluir.libeditor
    "assets/fonts/DejaVuSansMono.ttf"
    fluir::editor::assets::dejaVuSansMono
)

target_link_libraries(
    fluir.libeditor
    PUBLIC fmt::fmt
           SDL3::SDL3
           SDL3_ttf::SDL3_ttf
           nfd
           fluir::compiler
)

add_executable(fluir.editor "main.cpp")

target_link_libraries(fluir.editor PUBLIC fluir.libeditor)

turn_up_warnings_on(fluir.editor)
