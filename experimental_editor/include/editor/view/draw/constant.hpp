#pragma once

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::draw {

  PortSet anchors(const pt::Constant& node, const Rect& world, const EditorContext::Layout& layout);

  /** Colored by the literal's type family: float, signed or unsigned. */
  Color color(const pt::Constant& node, const EditorContext::Theme& theme);

  /** The bool toggle square in world space: left of the move grip, vertically centred. Empty for a node
   *  too narrow to hold it. */
  Rect boolToggleRect(const Rect& world, const EditorContext::Layout& layout);

  /** Draws the body only; selection outlines belong to the caller. */
  void draw(const pt::Constant& node, const Rect& world, const Subview& view, const EditorContext& ctx);

}  // namespace fluir::editor::draw
