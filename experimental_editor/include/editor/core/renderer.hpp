#pragma once

#include <cstddef>
#include <string_view>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** Monospace cell width of the UI font, in screen px. Tools map clicks with it, having no renderer. */
  inline constexpr double GLYPH_PX = 8.0;

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
    /** Glyphs are UI size x `scale`. */
    virtual void drawText(Vec2 topLeft, std::string_view text, const Color& color, double scale = 1.0) = 0;
    /** Text from `screen`'s top-left, wrapped at `screen.w`; glyphs are UI size x `scale`. Caller clips. */
    virtual void drawTextWrapped(Rect screen, std::string_view text, double scale, const Color& color) = 0;
    /** Caret byte index nearest `point` in `text` laid out as `drawTextWrapped` would. */
    virtual std::size_t wrappedIndexAt(Rect screen, std::string_view text, double scale, Vec2 point) = 0;
    /** 1 px wide caret before byte `index` of `text` laid out as `drawTextWrapped` would. */
    virtual Rect wrappedCaretRect(Rect screen, std::string_view text, double scale, std::size_t index) = 0;

    /** Screen-px size `text` would occupy if drawn. */
    virtual Vec2 measureText(std::string_view text) = 0;
    virtual void pushClip(Rect screen) = 0;
    virtual void popClip() = 0;
  };

}  // namespace fluir::editor
