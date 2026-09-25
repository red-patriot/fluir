#ifndef FLUIR_EDITOR_VIEW_DRAW_CALL_HPP
#define FLUIR_EDITOR_VIEW_DRAW_CALL_HPP

#include <optional>
#include <vector>

#include "editor/core/tree.hpp"
#include "editor/view/draw/draw_utils.hpp"
#include "editor/view/graph_layout.hpp"

namespace fluir::editor::draw {

  /** One input per argument row; an output only when the call returns. */
  TerminalSet anchors(const et::Call& node, const Rect& world, const EditorContext::Layout& layout);

  Color color(const et::Call& node, const EditorContext::Theme& theme);

  /** How far the node may be resized, in grid units. */
  Limits<Vec2i> sizeLimits(const et::Call& node);

  /** The grip that resizes the node, if any. */
  std::optional<Part> resizePart(const et::Call& node);

  /** The target on the header row, then one row per argument in index order. */
  std::vector<FieldLabel> labels(const et::Call& node, const Rect& world, const EditorContext::Layout& layout);

  /** Draws the body only; selection outlines belong to the caller. */
  void draw(const et::Call& node, const Rect& world, const Subview& view, const EditorContext& ctx);

}  // namespace fluir::editor::draw

#endif
