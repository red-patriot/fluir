#include <cstdio>
#include <exception>

#include <fmt/format.h>
#include <nfd.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

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

  if (!TTF_Init()) {
    fmt::print(stderr, "TTF_Init failed: {}\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  if (NFD_Init() != NFD_OKAY) {
    fmt::print(stderr, "NFD_Init failed: {}\n", NFD_GetError());
    TTF_Quit();
    SDL_Quit();
    return 1;
  }

  SDL_Window* window = SDL_CreateWindow("Fluir", editorCtx.window.width, editorCtx.window.height, SDL_WINDOW_RESIZABLE);
  if (!window) {
    fmt::print(stderr, "SDL_CreateWindow failed: {}\n", SDL_GetError());
    NFD_Quit();
    TTF_Quit();
    SDL_Quit();
    return 1;
  }

  SDL_StartTextInput(window);

  SDL_Renderer* sdl = SDL_CreateRenderer(window, nullptr);
  if (!sdl) {
    fmt::print(stderr, "SDL_CreateRenderer failed: {}\n", SDL_GetError());
    SDL_DestroyWindow(window);
    NFD_Quit();
    TTF_Quit();
    SDL_Quit();
    return 1;
  }

  int rc = 1;
  try {
    // Scoped so the renderer's text engine is destroyed before the SDL_Renderer.
    fluir::editor::SdlRenderer renderer{sdl, editorCtx.theme};
    rc = fluir::editor::run(editorCtx, renderer);
  } catch (const std::exception& e) {
    fmt::print(stderr, "Renderer failed: {}\n", e.what());
  }

  SDL_DestroyRenderer(sdl);
  SDL_DestroyWindow(window);
  NFD_Quit();
  TTF_Quit();
  SDL_Quit();
  return rc;
}
