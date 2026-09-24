#ifndef FLUIR_EDITOR_VIEW_DRAW_BINARY_HPP
#define FLUIR_EDITOR_VIEW_DRAW_BINARY_HPP

#include <optional>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/view/draw/draw_utils.hpp"
#include "editor/view/graph_layout.hpp"

namespace fluir::editor::draw {

  TerminalSet anchors(const pt::Binary& node, const Rect& world, const EditorContext::Layout& layout);

  Color color(const pt::Binary& node, const EditorContext::Theme& theme);

  /** How far the node may be resized, in grid units. */
  Limits<Vec2i> sizeLimits(const pt::Binary& node);

  /** The grip that resizes the node, if any. */
  std::optional<Part> resizePart(const pt::Binary& node);

  /** The operator, over the whole node. */
  std::vector<FieldLabel> labels(const pt::Binary& node, const Rect& world, const EditorContext::Layout& layout);

  /** Draws the body only; selection outlines belong to the caller. */
  void draw(const pt::Binary& node, const Rect& world, const Subview& view, const EditorContext& ctx);

}  // namespace fluir::editor::draw

#endif
