#pragma once

#include <string_view>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  /** A node's port anchors, in world space. */
  struct PortSet {
    std::vector<Vec2> inputs;
    std::vector<Vec2> outputs;
  };

  /** `node`'s port anchors, given its world rect. */
  PortSet ports(const pt::Node& node, Rect world, const EditorContext::Layout& layout);

  /** Draws `node`'s body at `world`: fill, border, label and port dots. */
  void drawNode(const pt::Node& node, Rect world, const Subview& view, const EditorContext& ctx);

  /** Draws a top-level `comment` at `world`. */
  void drawComment(const pt::Comment& comment, Rect world, const Subview& view, const EditorContext& ctx);

  /** Where a comment at `world` wraps its text. */
  Rect commentTextRect(Rect world, const EditorContext::Layout& layout);

  /** A label's small tag and main text regions in world space; glyphs are fixed screen px, so the split depends on
   *  `viewScale`. */
  struct SplitLabel {
    Rect tag;
    Rect text;
  };

  SplitLabel splitLabel(Rect box, std::string_view tag, double viewScale, const EditorContext::Layout& layout);

  /** Draws `tag` small along `box`'s bottom and, when non-empty, `text` full size after it. */
  void drawSplitLabel(
    const Subview& view, Rect box, std::string_view tag, std::string_view text, const EditorContext& ctx);

  /** `node`'s fill color. */
  Color nodeColor(const pt::Node& node, const EditorContext::Theme& theme);

}  // namespace fluir::editor
