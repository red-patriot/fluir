#include <cstdio>

#include <fmt/format.h>
#include <nfd.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "editor/app.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/sdl_renderer.hpp"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  fluir::editor::EditorContext editorCtx;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    fmt::print(stderr, "SDL_Init failed: {}\n", SDL_GetError());
    return 1;
  }

  if (NFD_Init() != NFD_OKAY) {
    fmt::print(stderr, "NFD_Init failed: {}\n", NFD_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_Window* window = SDL_CreateWindow("Fluir", editorCtx.window.width, editorCtx.window.height, SDL_WINDOW_RESIZABLE);
  if (!window) {
    fmt::print(stderr, "SDL_CreateWindow failed: {}\n", SDL_GetError());
    NFD_Quit();
    SDL_Quit();
    return 1;
  }

  SDL_StartTextInput(window);

  SDL_Renderer* sdl = SDL_CreateRenderer(window, nullptr);
  if (!sdl) {
    fmt::print(stderr, "SDL_CreateRenderer failed: {}\n", SDL_GetError());
    SDL_DestroyWindow(window);
    NFD_Quit();
    SDL_Quit();
    return 1;
  }

  fluir::editor::SdlRenderer renderer{sdl, editorCtx.theme};
  const int rc = fluir::editor::run(editorCtx, renderer);

  SDL_DestroyRenderer(sdl);
  SDL_DestroyWindow(window);
  NFD_Quit();
  SDL_Quit();
  return rc;
}
