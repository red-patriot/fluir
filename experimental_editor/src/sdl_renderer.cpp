#include "editor/sdl_renderer.hpp"

#include <algorithm>
#include <cmath>
#include <span>
#include <stdexcept>
#include <string>

#include "editor/assets/fonts.hpp"

namespace fluir::editor {

  namespace {

    // Size the UI font is probed at to find the px size whose advance is GLYPH_PX.
    constexpr double kProbePx = 16.0;

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
    engine_ = TTF_CreateRendererTextEngine(renderer_);
    if (engine_ == nullptr) {
      throw std::runtime_error(SDL_GetError());
    }
    try {
      int advance = 0;
      if (!TTF_GetStringSize(fontAt(kProbePx), "0", 1, &advance, nullptr) || advance <= 0) {
        throw std::runtime_error(SDL_GetError());
      }
      uiPx_ = kProbePx * GLYPH_PX / advance;
      fontAt(uiPx_);
    } catch (...) {
      releaseText();
      throw;
    }
  }

  SdlRenderer::~SdlRenderer() { releaseText(); }

  void SdlRenderer::releaseText() {
    for (const auto& [key, font] : fonts_) {
      TTF_CloseFont(font);
    }
    fonts_.clear();
    if (engine_ != nullptr) {
      TTF_DestroyRendererTextEngine(engine_);
      engine_ = nullptr;
    }
  }

  TTF_Font* SdlRenderer::fontAt(double px) {
    const int key = std::max(1, static_cast<int>(std::lround(px * 2.0)));
    if (const auto it = fonts_.find(key); it != fonts_.end()) {
      return it->second;
    }
    const std::span<const unsigned char> bytes = assets::dejaVuSansMono();
    SDL_IOStream* io = SDL_IOFromConstMem(bytes.data(), bytes.size());
    TTF_Font* font = io == nullptr ? nullptr : TTF_OpenFontIO(io, true, static_cast<float>(key) / 2.0f);
    if (font == nullptr) {
      throw std::runtime_error(SDL_GetError());
    }
    fonts_.emplace(key, font);
    return font;
  }

  void SdlRenderer::drawTtf(TTF_Font* font, Vec2 topLeft, std::string_view text, int wrapWidth, const Color& color) {
    TTF_Text* t = TTF_CreateText(engine_, font, text.data(), text.size());
    if (t == nullptr) {
      return;
    }
    TTF_SetTextWrapWidth(t, wrapWidth);
    TTF_SetTextColor(t, color.r, color.g, color.b, color.a);
    TTF_DrawRendererText(t, static_cast<float>(topLeft.x), static_cast<float>(topLeft.y));
    TTF_DestroyText(t);
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
    if (!text.empty()) {
      drawTtf(fontAt(uiPx_), topLeft, text, 0, color);
    }
  }

  void SdlRenderer::drawTextWrapped(Rect screen, std::string_view text, double scale, const Color& color) {
    if (screen.w < 1.0 || text.empty() || scale <= 0.0) {
      return;
    }
    drawTtf(fontAt(uiPx_ * scale), screen.topLeft(), text, static_cast<int>(std::lround(screen.w)), color);
  }

  Vec2 SdlRenderer::measureText(std::string_view text) {
    int w = 0;
    int h = 0;
    TTF_GetStringSize(fontAt(uiPx_), text.data(), text.size(), &w, &h);
    return {static_cast<double>(w), static_cast<double>(h)};
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
