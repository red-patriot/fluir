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

include(strict-warnings)

add_library(fluir.libeditor)

target_include_directories(
    fluir.libeditor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

set(FLUIR_EDITOR_CORE_SOURCES
    "core/collecting_sink.cpp"
    "core/graph_geometry.cpp"
    "core/interaction.cpp"
    "core/layer.cpp"
    "core/loader.cpp"
    "core/parse_tree_writer.cpp"
    "core/render_graph.cpp"
    "core/viewport.cpp"
)

set(FLUIR_EDITOR_PAGE_SOURCES
    "pages/module.cpp"
    "pages/header_bar.cpp"
    "pages/splash.cpp"
)

set(FLUIR_EDITOR_ACTOR_SOURCES
    "actors/function_decl_actor.cpp"
    "actors/node_actor.cpp"
    "actors/node_actors.cpp"
    "actors/scene.cpp"
)

set(FLUIR_EDITOR_COMPONENT_SOURCES
    "components/button_actor.cpp"
    "components/drag_handle.cpp"
    "components/toolbar_actor.cpp"
)

target_sources(
    fluir.libeditor
    PRIVATE ${FLUIR_EDITOR_CORE_SOURCES}
            ${FLUIR_EDITOR_PAGE_SOURCES}
            ${FLUIR_EDITOR_ACTOR_SOURCES}
            ${FLUIR_EDITOR_COMPONENT_SOURCES}
            "sdl_renderer.cpp"
            "input.cpp"
            "app.cpp"
)

target_link_libraries(
    fluir.libeditor
    PUBLIC fmt::fmt
           SDL3::SDL3
           nfd
           fluir::compiler
)

add_executable(fluir.editor "main.cpp")

target_link_libraries(fluir.editor PUBLIC fluir.libeditor)

turn_up_warnings_on(fluir.editor)
