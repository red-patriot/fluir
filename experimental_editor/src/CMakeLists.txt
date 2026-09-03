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

include(strict-warnings)

add_library(fluir.libeditor)

target_include_directories(
    fluir.libeditor PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

set(FLUIR_EDITOR_CORE_SOURCES
    "core/collecting_sink.cpp"
    "core/loader.cpp"
    "core/render_graph.cpp"
    "core/viewport.cpp"
)

set(FLUIR_EDITOR_PAGE_SOURCES "pages/module.cpp")

target_sources(
    fluir.libeditor
    PRIVATE ${FLUIR_EDITOR_CORE_SOURCES}
            ${FLUIR_EDITOR_PAGE_SOURCES}
            "sdl_renderer.cpp"
            "input.cpp"
            "app.cpp"
)

target_link_libraries(
    fluir.libeditor
    PUBLIC fmt::fmt
           SDL3::SDL3
           fluir::compiler
)

add_executable(fluir.editor "main.cpp")

target_link_libraries(fluir.editor PUBLIC fluir.libeditor)

turn_up_warnings_on(fluir.editor)
