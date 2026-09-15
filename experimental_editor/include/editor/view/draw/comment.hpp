#pragma once

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor {

  /** Where a comment at `world` wraps its text. */
  Rect commentTextRect(Rect world, const EditorContext::Layout& layout);

  namespace draw {

    /** Comments have no ports. */
    PortSet anchors(const pt::Comment& node, const Rect& world, const EditorContext::Layout& layout);

    Color color(const pt::Comment& node, const EditorContext::Theme& theme);

    /** Draws the body only; selection outlines belong to the caller. */
    void draw(const pt::Comment& node, const Rect& world, const Subview& view, const EditorContext& ctx);

  }  // namespace draw

}  // namespace fluir::editor
