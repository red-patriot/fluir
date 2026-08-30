#include <iostream>

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

  /* No event handling. Run a fixed number of frames, then exit.
     Without an event pump the window cannot be closed by the user
     and the window manager may flag it "not responding". */
  for (int frame = 0; frame < 600; ++frame) {
    const double now = ((double)SDL_GetTicks()) / 1000.0; /* convert from milliseconds to seconds. */
    /* choose the color for the frame we will draw. The sine wave trick makes it fade between colors smoothly. */
    const float red = (float)(0.5 + 0.5 * SDL_sin(now));
    const float green = (float)(0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
    const float blue = (float)(0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));
    SDL_SetRenderDrawColorFloat(renderer, red, green, blue, SDL_ALPHA_OPAQUE_FLOAT); /* new color, full alpha. */

    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    SDL_PumpEvents();
    SDL_Delay(16);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
