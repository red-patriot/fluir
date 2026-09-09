#pragma once

#include <string_view>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** Sink for drawing primitives, in screen-space pixels. */
  class Renderer {
   public:
    virtual ~Renderer() = default;

    /** Called once before drawing to set up for drawing the next frame*/
    virtual void beginFrame() = 0;
    /** Called once at the end of drawing to present the frame */
    virtual void endFrame() = 0;

    /** Drawable size in screen px (device px). */
    virtual Vec2 outputSize() = 0;

    virtual void drawRect(Rect screen, const Color& color) = 0;
    virtual void fillRect(Rect screen, const Color& color) = 0;
    virtual void drawLine(Vec2 a, Vec2 b, const Color& color) = 0;
    virtual void drawText(Vec2 topLeft, std::string_view text, const Color& color) = 0;

    /** Screen-px size `text` would occupy if drawn. */
    virtual Vec2 measureText(std::string_view text) = 0;
    virtual void pushClip(Rect screen) = 0;
    virtual void popClip() = 0;
  };

}  // namespace fluir::editor
