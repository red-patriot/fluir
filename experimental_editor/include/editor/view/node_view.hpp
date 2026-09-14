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

  /** A rail's type and name regions in world space; glyphs are fixed screen px, so the split depends on `viewScale`. */
  struct RailLabels {
    Rect type;
    Rect name;
  };

  RailLabels railLabels(Rect rail, std::string_view label, double viewScale, const EditorContext::Layout& layout);

  /** Draws a rail's small bottom-aligned `type` and, when non-empty, its `name` after it. */
  void drawRailLabels(
    const Subview& view, Rect rail, std::string_view type, std::string_view name, const EditorContext& ctx);

  /** `node`'s fill color. */
  Color nodeColor(const pt::Node& node, const EditorContext::Theme& theme);

}  // namespace fluir::editor
