#ifndef FLUIR_EDITOR_VIEW_NODE_VIEW_HPP
#define FLUIR_EDITOR_VIEW_NODE_VIEW_HPP

#include <optional>
#include <vector>

#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/tree.hpp"
#include "editor/core/viewport.hpp"
#include "editor/view/draw/draw_utils.hpp"
#include "editor/view/graph_layout.hpp"

namespace fluir::editor {

  /** `node`'s terminal anchors, given its world rect. */
  TerminalSet terminals(const et::Node& node, Rect world, const EditorContext::Layout& layout);

  /** Where `node`'s editable fields draw, given its world rect. */
  std::vector<FieldLabel> nodeLabels(const et::Node& node, Rect world, const EditorContext::Layout& layout);

  /** How far `node` may be resized, in grid units. */
  Limits<Vec2i> nodeSizeLimits(const et::Node& node);

  /** The grip that resizes `node`, if any. */
  std::optional<Part> nodeResizePart(const et::Node& node);

  /** Draws `node`'s body at `world`: fill, border, label and terminal dots. */
  void drawNode(const et::Node& node, Rect world, const Subview& view, const EditorContext& ctx);

  /** `node`'s fill color. */
  Color nodeColor(const et::Node& node, const EditorContext::Theme& theme);

}  // namespace fluir::editor

#endif
