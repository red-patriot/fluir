#ifndef FLUIR_EDITOR_VIEW_DRAW_CALL_HPP
#define FLUIR_EDITOR_VIEW_DRAW_CALL_HPP

#include <optional>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::draw {

  /** One input per argument row; an output only when the call returns. */
  TerminalSet anchors(const pt::Call& node, const Rect& world, const EditorContext::Layout& layout);

  Color color(const pt::Call& node, const EditorContext::Theme& theme);

  /** How far the node may be resized, in grid units. */
  Limits<Vec2i> sizeLimits(const pt::Call& node);

  /** The size a new call starts at, in grid units: a row for its target and one per argument. */
  Vec2i defaultSize(const pt::Call& node);

  /** The grip that resizes the node, if any. */
  std::optional<ResizeAxes> resizePart(const pt::Call& node);

  /** The target on the header row, then one row per argument in index order. */
  std::vector<FieldLabel> labels(const pt::Call& node, const Rect& world, const EditorContext::Layout& layout);

}  // namespace fluir::editor::draw

#endif
