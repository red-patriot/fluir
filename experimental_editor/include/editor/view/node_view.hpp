#pragma once

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor {

  /** `node`'s terminal anchors, given its world rect. */
  TerminalSet terminals(const pt::Node& node, Rect world, const EditorContext::Layout& layout);

  /** Draws `node`'s body at `world`: fill, border, label and terminal dots. */
  void drawNode(const pt::Node& node, Rect world, const Subview& view, const EditorContext& ctx);

  /** `node`'s fill color. */
  Color nodeColor(const pt::Node& node, const EditorContext::Theme& theme);

}  // namespace fluir::editor
