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

add_executable(fluir.editor "main.cpp")

target_link_libraries(fluir.editor PUBLIC SDL3::SDL3 fmt::fmt)

turn_up_warnings_on(fluir.editor)
