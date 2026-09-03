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

  SdlRenderer::SdlRenderer(SDL_Renderer* renderer, EditorContext::Theme theme) : renderer_(renderer), theme_(theme) {
    applyScale();
  }

  void SdlRenderer::setColor(Color c) { SDL_SetRenderDrawColor(renderer_, c.r, c.g, c.b, c.a); }

  void SdlRenderer::applyScale() {
    SDL_Window* w = SDL_GetRenderWindow(renderer_);
    float dpi = SDL_GetWindowDisplayScale(w);
    if (dpi <= 0.0f) {
      dpi = 1.0f;
    }
    dpi_ = dpi;
    SDL_SetRenderScale(renderer_, dpi_, dpi_);
  }

  Vec2 SdlRenderer::outputSize() {
    applyScale();
    int w = 0;
    int h = 0;
    SDL_GetCurrentRenderOutputSize(renderer_, &w, &h);
    return {w / dpi_, h / dpi_};
  }

  void SdlRenderer::beginFrame() {
    setColor(theme_.background);
    SDL_RenderClear(renderer_);
  }

  void SdlRenderer::endFrame() { SDL_RenderPresent(renderer_); }

  void SdlRenderer::drawRect(Rect screen, const Color& color) {
    const SDL_FRect r = toFRect(screen);
    setColor(color);
    SDL_RenderRect(renderer_, &r);
  }

  void SdlRenderer::fillRect(Rect screen, const Color& color) {
    const SDL_FRect r = toFRect(screen);
    setColor(color);
    SDL_RenderFillRect(renderer_, &r);
  }

  void SdlRenderer::drawLine(Vec2 a, Vec2 b, const Color& color) {
    setColor(color);
    SDL_RenderLine(
      renderer_, static_cast<float>(a.x), static_cast<float>(a.y), static_cast<float>(b.x), static_cast<float>(b.y));
  }

  void SdlRenderer::drawText(Vec2 topLeft, std::string_view text, const Color& color) {
    // SDL's built-in 8px debug font. Fixed screen size (does not scale with
    // zoom); a real glyph atlas is a later phase.
    setColor(color);
    const std::string str{text};
    SDL_RenderDebugText(renderer_, static_cast<float>(topLeft.x), static_cast<float>(topLeft.y), str.c_str());
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
