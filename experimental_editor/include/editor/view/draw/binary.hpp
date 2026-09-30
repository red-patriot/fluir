#ifndef FLUIR_EDITOR_VIEW_DRAW_BINARY_HPP
#define FLUIR_EDITOR_VIEW_DRAW_BINARY_HPP

#include <optional>
#include <vector>

#include "editor/core/tree.hpp"
#include "editor/view/draw/draw_utils.hpp"
#include "editor/view/graph_layout.hpp"

namespace fluir::editor::draw {

  TerminalSet anchors(const et::Binary& node, const Rect& world, const EditorContext::Layout& layout);

  Color color(const et::Binary& node, const EditorContext::Theme& theme);

  /** How far the node may be resized, in grid units. */
  Limits<Vec2i> sizeLimits(const et::Binary& node);

  /** The grip that resizes the node, if any. */
  std::optional<Part> resizePart(const et::Binary& node);

  /** The operator, over the whole node. */
  std::vector<FieldLabel> labels(const et::Binary& node, const Rect& world, const EditorContext::Layout& layout);

  /** Draws the body only; selection outlines belong to the caller. */
  void draw(const et::Binary& node, const Rect& world, const Subview& view, const EditorContext& ctx);

}  // namespace fluir::editor::draw

#endif
