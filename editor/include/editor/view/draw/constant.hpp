#ifndef FLUIR_EDITOR_VIEW_DRAW_CONSTANT_HPP
#define FLUIR_EDITOR_VIEW_DRAW_CONSTANT_HPP

#include <optional>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::draw {

  TerminalSet anchors(const pt::Constant& node, const Rect& world, const EditorContext::Layout& layout);

  /** Colored by the literal's type family: float, signed or unsigned. */
  Color color(const pt::Constant& node, const EditorContext::Theme& theme);

  /** How far the node may be resized, in grid units. */
  Limits<Vec2i> sizeLimits(const pt::Constant& node);

  /** The size a new constant starts at, in grid units; a bool is only its toggle. */
  Vec2i defaultSize(const pt::Constant& node);

  /** The grip that resizes the node, if any. */
  std::optional<ResizeAxes> resizePart(const pt::Constant& node);

  /** The bool toggle square in world space: left of the move grip, vertically centred. Empty for a node
   *  too narrow to hold it. */
  Rect boolToggleRect(const Rect& world, const EditorContext::Layout& layout);

  /** A bool's toggle square, or any other literal's text after its type tag. */
  std::vector<FieldLabel> labels(const pt::Constant& node, const Rect& world, const EditorContext::Layout& layout);

}  // namespace fluir::editor::draw

#endif
