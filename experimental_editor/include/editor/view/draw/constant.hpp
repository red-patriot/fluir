#ifndef FLUIR_EDITOR_VIEW_DRAW_CONSTANT_HPP
#define FLUIR_EDITOR_VIEW_DRAW_CONSTANT_HPP

#include <optional>
#include <vector>

#include "editor/core/tree.hpp"
#include "editor/view/draw/draw_utils.hpp"
#include "editor/view/graph_layout.hpp"

namespace fluir::editor::draw {

  TerminalSet anchors(const et::Constant& node, const Rect& world, const EditorContext::Layout& layout);

  /** Colored by the literal's type family: float, signed or unsigned. */
  Color color(const et::Constant& node, const EditorContext::Theme& theme);

  /** How far the node may be resized, in grid units. */
  Limits<Vec2i> sizeLimits(const et::Constant& node);

  /** The grip that resizes the node, if any. */
  std::optional<Part> resizePart(const et::Constant& node);

  /** The bool toggle square in world space: left of the move grip, vertically centred. Empty for a node
   *  too narrow to hold it. */
  Rect boolToggleRect(const Rect& world, const EditorContext::Layout& layout);

  /** A bool's toggle square, or any other literal's text after its type tag. */
  std::vector<FieldLabel> labels(const et::Constant& node, const Rect& world, const EditorContext::Layout& layout);

  /** Draws the body only; selection outlines belong to the caller. */
  void draw(const et::Constant& node, const Rect& world, const Subview& view, const EditorContext& ctx);

}  // namespace fluir::editor::draw

#endif
