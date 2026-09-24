#ifndef FLUIR_EDITOR_VIEW_DRAW_COMMENT_HPP
#define FLUIR_EDITOR_VIEW_DRAW_COMMENT_HPP

#include <optional>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/view/draw/draw_utils.hpp"
#include "editor/view/graph_layout.hpp"

namespace fluir::editor {

  /** A comment's region below its header band. */
  Rect commentBodyRect(Rect world, const EditorContext::Layout& layout);

  /** Where a comment wraps its text, given the comment's body rect. */
  Rect commentTextRect(Rect body, const EditorContext::Layout& layout);

  namespace draw {

    /** Comments have no terminals. */
    TerminalSet anchors(const pt::Comment& node, const Rect& world, const EditorContext::Layout& layout);

    Color color(const pt::Comment& node, const EditorContext::Theme& theme);

    /** How far the comment may be resized, in grid units. */
    Limits<Vec2i> sizeLimits(const pt::Comment& node);

    /** The grip that resizes the comment, if any. */
    std::optional<Part> resizePart(const pt::Comment& node);

    /** The text, over the body below the header band. */
    std::vector<FieldLabel> labels(const pt::Comment& node, const Rect& world, const EditorContext::Layout& layout);

    /** Draws the body only; selection outlines belong to the caller. */
    void draw(const pt::Comment& node, const Rect& world, const Subview& view, const EditorContext& ctx);
  }  // namespace draw

}  // namespace fluir::editor

#endif
