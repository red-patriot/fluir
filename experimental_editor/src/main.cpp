#include <fmt/format.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    fmt::print("SDL_Init failed: {}", SDL_GetError());
    return 1;
  }

  SDL_Window* window = SDL_CreateWindow("fluir", 640, 480, SDL_WINDOW_RESIZABLE);
  if (!window) {
    fmt::print("SDL_CreateWindow failed: {}", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
  if (!renderer) {
    fmt::print("SDL_CreateRenderer failed: {}", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  // A viewer is idle most of the time, so block until something happens
  // instead of spinning a frame loop. Each wake redraws once.
  bool running = true;
  while (running) {
    SDL_Event event;
    if (!SDL_WaitEvent(&event)) {
      fmt::print("SDL_WaitEvent failed: {}", SDL_GetError());
      break;
    }

    if (event.type == SDL_EVENT_QUIT) {
      running = false;
    } else if (event.type == SDL_EVENT_KEY_DOWN &&
               (event.key.scancode == SDL_SCANCODE_ESCAPE || event.key.key == SDLK_ESCAPE)) {
      running = false;
    }

    SDL_SetRenderDrawColorFloat(renderer, 0.10f, 0.11f, 0.13f, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
