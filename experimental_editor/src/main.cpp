#include <cstdio>
#include <filesystem>

#include <fmt/format.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "compiler/utility/context.hpp"
#include "editor/app.hpp"
#include "editor/core/collecting_sink.hpp"
#include "editor/core/loader.hpp"
#include "editor/sdl_renderer.hpp"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fmt::print(stderr, "usage: {} <file.fl>\n", argv[0]);
    return 2;
  }

  const std::filesystem::path path = argv[1];

  fluir::editor::CollectingSink sink;
  fluir::Context ctx{
    .diagnosticSink = sink,
    .symbolTable = {},
    .currentFile = {},
    .outputFilename = {},
    .version = {},
    .ignoreVersionChecks = true,
  };

  const auto result = fluir::editor::loadFile(ctx, path);
  for (const auto& message : sink.messages()) {
    fmt::print(stderr, "{}\n", message);
  }
  if (!result.tree) {
    fmt::print(stderr, "parse failed: {}\n", path.string());
    return 1;
  }

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    fmt::print(stderr, "SDL_Init failed: {}\n", SDL_GetError());
    return 1;
  }

  SDL_Window* window = SDL_CreateWindow(path.filename().string().c_str(), 1280, 800, SDL_WINDOW_RESIZABLE);
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

  fluir::editor::SdlRenderer renderer{sdl};
  const int rc = fluir::editor::run(*result.tree, renderer, renderer.outputSize());

  SDL_DestroyRenderer(sdl);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return rc;
}
