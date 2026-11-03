#ifndef FLUIR_EDITOR_VIEW_DRAW_UNARY_HPP
#define FLUIR_EDITOR_VIEW_DRAW_UNARY_HPP

#include <optional>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::draw {

  TerminalSet anchors(const pt::Unary& node, const Rect& world, const EditorContext::Layout& layout);

  Color color(const pt::Unary& node, const EditorContext::Theme& theme);

  /** How far the node may be resized, in grid units. */
  Limits<Vec2i> sizeLimits(const pt::Unary& node);

  /** The size a new unary starts at, in grid units. */
  Vec2i defaultSize(const pt::Unary& node);

  /** The grip that resizes the node, if any. */
  std::optional<ResizeAxes> resizePart(const pt::Unary& node);

  /** The operator, over the whole node. */
  std::vector<FieldLabel> labels(const pt::Unary& node, const Rect& world, const EditorContext::Layout& layout);

}  // namespace fluir::editor::draw

#endif
