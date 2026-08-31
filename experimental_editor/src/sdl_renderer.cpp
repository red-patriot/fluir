#include "editor/sdl_renderer.hpp"

#include <cmath>
#include <string>

namespace fluir::editor {

  namespace {

    SDL_FRect toFRect(Rect r) {
      return SDL_FRect{
        static_cast<float>(r.x),
        static_cast<float>(r.y),
        static_cast<float>(r.w),
        static_cast<float>(r.h),
      };
    }

    SDL_Rect toIRect(Rect r) {
      const auto x0 = static_cast<int>(std::lround(r.x));
      const auto y0 = static_cast<int>(std::lround(r.y));
      const auto x1 = static_cast<int>(std::lround(r.x + r.w));
      const auto y1 = static_cast<int>(std::lround(r.y + r.h));
      return SDL_Rect{x0, y0, x1 - x0, y1 - y0};
    }

  }  // namespace

  void SdlRenderer::beginFrame() {
    SDL_SetRenderDrawColor(renderer_, 24, 26, 31, 255);
    SDL_RenderClear(renderer_);
  }

  void SdlRenderer::endFrame() { SDL_RenderPresent(renderer_); }

  void SdlRenderer::drawRect(Rect screen) {
    const SDL_FRect r = toFRect(screen);
    SDL_SetRenderDrawColor(renderer_, 200, 200, 210, 255);
    SDL_RenderRect(renderer_, &r);
  }

  void SdlRenderer::fillRect(Rect screen) {
    const SDL_FRect r = toFRect(screen);
    SDL_SetRenderDrawColor(renderer_, 120, 120, 140, 255);
    SDL_RenderFillRect(renderer_, &r);
  }

  void SdlRenderer::drawLine(Vec2 a, Vec2 b) {
    SDL_SetRenderDrawColor(renderer_, 150, 180, 220, 255);
    SDL_RenderLine(
      renderer_, static_cast<float>(a.x), static_cast<float>(a.y), static_cast<float>(b.x), static_cast<float>(b.y));
  }

  void SdlRenderer::drawText(Vec2 topLeft, std::string_view text) {
    (void)topLeft;
    (void)text;
  }

  void SdlRenderer::pushClip(Rect screen) {
    SDL_Rect rect = toIRect(screen);
    if (!clipStack_.empty()) {
      SDL_Rect clipped{};
      if (!SDL_GetRectIntersection(&rect, &clipStack_.back(), &clipped)) {
        clipped = SDL_Rect{rect.x, rect.y, 0, 0};
      }
      rect = clipped;
    }
    clipStack_.push_back(rect);
    SDL_SetRenderClipRect(renderer_, &clipStack_.back());
  }

  void SdlRenderer::popClip() {
    if (clipStack_.empty()) {
      return;
    }
    clipStack_.pop_back();
    if (clipStack_.empty()) {
      SDL_SetRenderClipRect(renderer_, nullptr);
    } else {
      SDL_SetRenderClipRect(renderer_, &clipStack_.back());
    }
  }

}  // namespace fluir::editor
