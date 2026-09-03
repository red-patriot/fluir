#pragma once

#include <string_view>
#include <vector>

#include <SDL3/SDL.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/renderer.hpp"

namespace fluir::editor {

  /** Draws to an SDL_Renderer. Does NOT own it. Screen-space px, matching the
   *  Renderer contract. Text is a no-op until the glyph atlas (P2b). */
  class SdlRenderer final : public Renderer {
   public:
    SdlRenderer(SDL_Renderer* renderer, EditorContext::Theme theme);

    void beginFrame() override;
    void endFrame() override;
    Vec2 outputSize() override;
    void drawRect(Rect screen) override;
    void fillRect(Rect screen) override;
    void drawLine(Vec2 a, Vec2 b) override;
    void drawText(Vec2 topLeft, std::string_view text) override;  // no-op for now
    void pushClip(Rect screen) override;
    void popClip() override;

   private:
    /** Apply the display scale via SDL_SetRenderScale.
     *  Idempotent and responds automatically to a monitor/scale change. */
    void applyScale();

    /** Set the SDL draw color from a theme Color. */
    void setColor(Color c);

    SDL_Renderer* renderer_;
    EditorContext::Theme theme_;
    float dpi_ = 1.0f;
    std::vector<SDL_Rect> clipStack_;
  };

}  // namespace fluir::editor
