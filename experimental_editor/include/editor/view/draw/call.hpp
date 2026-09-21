#pragma once

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::draw {

  /** One input per argument row; an output only when the call returns. */
  TerminalSet anchors(const pt::Call& node, const Rect& world, const EditorContext::Layout& layout);

  Color color(const pt::Call& node, const EditorContext::Theme& theme);

  /** Draws the body only; selection outlines belong to the caller. */
  void draw(const pt::Call& node, const Rect& world, const Subview& view, const EditorContext& ctx);

}  // namespace fluir::editor::draw
