#pragma once

#include <string_view>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** Sink for drawing primitives, in screen-space pixels.. */
  class Renderer {
   public:
    virtual ~Renderer() = default;

    virtual void beginFrame() = 0;  ///< clear the target
    virtual void endFrame() = 0;    ///< present

    /** Drawable size in screen px (device px / dpi). */
    virtual Vec2 outputSize() = 0;

    virtual void drawRect(Rect screen, const Color& color) = 0;
    virtual void fillRect(Rect screen, const Color& color) = 0;
    virtual void drawLine(Vec2 a, Vec2 b, const Color& color) = 0;
    virtual void drawText(Vec2 topLeft, std::string_view text, const Color& color) = 0;
    virtual void pushClip(Rect screen) = 0;
    virtual void popClip() = 0;
  };

}  // namespace fluir::editor
