#ifndef FLUIR_EDITOR_VIEW_DRAW_DRAW_UTILS_HPP
#define FLUIR_EDITOR_VIEW_DRAW_DRAW_UTILS_HPP

#include <span>
#include <string_view>
#include <vector>

#include "editor/core/editor_context.hpp"
#include "editor/core/field.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** Estimated world px per monospace glyph. */
  inline constexpr double GLYPH_PX = 8.0;

  /** Which way a node's resize grip stretches it. */
  enum class ResizeAxes { X, XY };

  /** A node's terminal anchors, in world space. */
  struct TerminalSet {
    std::vector<Vec2> inputs;
    std::vector<Vec2> outputs;
  };

  /** How a field's value is shown: as main text, or as a small tag like a type. */
  enum class FieldLook { Text, Tag };

  /** Where `field`'s value draws and edits, in world space, and how it looks. */
  struct FieldLabel {
    Field field;
    Rect rect;
    FieldLook look = FieldLook::Text;
  };

  /** A label's small tag and main text regions in world space; glyph cells are world px, so the split does not depend
   *  on the view. */
  struct SplitLabel {
    Rect tag;
    Rect text;
  };

  /** The move grip over `frame`'s top-right corner, in the same space. */
  Rect moveGrip(const Rect& frame, double unit);

  /** The X resize grip: a thick line just inside `frame`'s right border, in the same space. */
  Rect resizeEdge(const Rect& frame, double unit);

  /** The XY resize grip in `frame`'s bottom-right corner, in the same space. */
  Rect resizeCorner(const Rect& frame, double unit);

  SplitLabel splitLabel(Rect box, std::string_view tag, const EditorContext::Layout& layout);

  /** `intrinsic`'s aspect scaled to fit inside `target`, centred. Degenerate input returns an empty rect at
   *  `target`'s centre. */
  Rect fitInto(Rect target, Vec2 intrinsic);

  namespace draw {

    /** `count` anchors down the vertical edge at `edgeX`; a lone one sits at the centre. */
    std::vector<Vec2> edgeAnchors(double edgeX, const Rect& rect, int count);

    /** The rect covering a node's selection outline: `frame` outset by the selection pad plus the 2px line. */
    Rect selectionOutline(const Rect& frame, const EditorContext::Layout& layout);

  }  // namespace draw

}  // namespace fluir::editor

#endif
