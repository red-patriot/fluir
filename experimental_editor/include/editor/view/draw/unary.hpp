#ifndef FLUIR_EDITOR_VIEW_DRAW_UNARY_HPP
#define FLUIR_EDITOR_VIEW_DRAW_UNARY_HPP

#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::draw {

  TerminalSet anchors(const pt::Unary& node, const Rect& world, const EditorContext::Layout& layout);

  Color color(const pt::Unary& node, const EditorContext::Theme& theme);

  /** The operator, over the whole node. */
  std::vector<FieldLabel> labels(const pt::Unary& node, const Rect& world, const EditorContext::Layout& layout);

  /** Draws the body only; selection outlines belong to the caller. */
  void draw(const pt::Unary& node, const Rect& world, const Subview& view, const EditorContext& ctx);

}  // namespace fluir::editor::draw

#endif
