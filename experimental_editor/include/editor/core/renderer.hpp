#ifndef FLUIR_EDITOR_CORE_RENDERER_HPP
#define FLUIR_EDITOR_CORE_RENDERER_HPP

#include <string_view>

#include "editor/assets/images.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/text_metrics.hpp"

namespace fluir::editor {

  /** Monospace cell width of the UI font, in renderer px. Graph callers pass the view scale, so for them it is a
   *  world-px cell. Tools map clicks with it, having no renderer. */
  inline constexpr double GLYPH_PX = 8.0;

  /** Sink for drawing primitives, in screen-space pixels. */
  class Renderer : public TextMetrics {
   public:
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
    /** Fills `screen` exactly with the SVG in `svg`, stretched to it and recolored to `tint`. Callers fit;
     *  this does not. */
    virtual void drawIcon(Rect screen, SvgView svg, const Color& tint) = 0;

    /** The SVG's intrinsic size, for aspect. */
    virtual Vec2 imageSize(SvgView svg) = 0;

    virtual void pushClip(Rect screen) = 0;
    virtual void popClip() = 0;
  };

}  // namespace fluir::editor

#endif
