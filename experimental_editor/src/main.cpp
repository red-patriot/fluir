#include <cstdio>

#include <fmt/format.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "editor/app.hpp"
#include "editor/core/editor_context.hpp"
// actors/actor.hpp (pulled in transitively) has unnamed-elsewhere default
// params that only trip -Wunused-parameter under this target's -Werror.
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "editor/pages/splash.hpp"
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
#include "editor/sdl_renderer.hpp"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  fluir::editor::EditorContext editorCtx;
  editorCtx.window.width = fluir::editor::SplashPage::kWidth;
  editorCtx.window.height = fluir::editor::SplashPage::kHeight;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    fmt::print(stderr, "SDL_Init failed: {}\n", SDL_GetError());
    return 1;
  }

  SDL_Window* window = SDL_CreateWindow("Fluir", editorCtx.window.width, editorCtx.window.height, SDL_WINDOW_RESIZABLE);
  if (!window) {
    fmt::print(stderr, "SDL_CreateWindow failed: {}\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_Renderer* sdl = SDL_CreateRenderer(window, nullptr);
  if (!sdl) {
    fmt::print(stderr, "SDL_CreateRenderer failed: {}\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  fluir::editor::SdlRenderer renderer{sdl, editorCtx.theme};
  const int rc = fluir::editor::run(editorCtx, renderer);

  SDL_DestroyRenderer(sdl);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return rc;
}
