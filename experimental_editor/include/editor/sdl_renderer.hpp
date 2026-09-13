#pragma once

#include <map>
#include <string_view>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/renderer.hpp"

namespace fluir::editor {

  /** Draws to an SDL_Renderer. Does NOT own it, and must be destroyed before it.
   *  Screen-space px, matching the Renderer contract. Text uses an embedded TTF; needs TTF_Init. */
  class SdlRenderer final : public Renderer {
   public:
    /** Throws std::runtime_error if the text engine or font cannot be created. */
    SdlRenderer(SDL_Renderer* renderer, EditorContext::Theme theme);
    ~SdlRenderer() override;

    SdlRenderer(const SdlRenderer&) = delete;
    SdlRenderer& operator=(const SdlRenderer&) = delete;

    void beginFrame() override;
    void endFrame() override;
    Vec2 outputSize() override;
    void drawRect(Rect screen, const Color& color) override;
    void fillRect(Rect screen, const Color& color) override;
    void drawLine(Vec2 a, Vec2 b, const Color& color) override;
    void drawText(Vec2 topLeft, std::string_view text, const Color& color) override;
    void drawTextWrapped(Rect screen, std::string_view text, double scale, const Color& color) override;
    Vec2 measureText(std::string_view text) override;
    void pushClip(Rect screen) override;
    void popClip() override;

   private:
    /** Apply the display scale via SDL_SetRenderScale.
     *  Idempotent and responds automatically to a monitor/scale change. */
    void applyScale();

    /** Set the SDL draw color. */
    void setColor(Color c);

    /** Close every font, then destroy the engine. */
    void releaseText();

    /** The embedded font at `px`, opened once per 0.5px bucket. */
    TTF_Font* fontAt(double px);

    /** Draw `text` at `topLeft` with `font`, wrapping at `wrapWidth` px (0 = no wrap). */
    void drawTtf(TTF_Font* font, Vec2 topLeft, std::string_view text, int wrapWidth, const Color& color);

    SDL_Renderer* renderer_;
    EditorContext::Theme theme_;
    float dpi_ = 1.0f;
    std::vector<SDL_Rect> clipStack_;
    TTF_TextEngine* engine_ = nullptr;
    std::map<int, TTF_Font*> fonts_;  ///< keyed by px x 2
    double uiPx_ = 16.0;
  };

}  // namespace fluir::editor
