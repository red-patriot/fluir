#pragma once

#include <string_view>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::draw {

  /** The small tag in a conditional's header label. */
  inline constexpr std::string_view IF_TAG = "if";

  /** None yet: a conditional wires through its scope ports, which is Phase 2. */
  PortSet anchors(const pt::Conditional& node, const Rect& world, const EditorContext::Layout& layout);

  Color color(const pt::Conditional& node, const EditorContext::Theme& theme);

  // A conditional paints in parts at different depths, so each part has its own draw.

  /** The body background, under the conditional's branches and their nodes. */
  void drawBody(const pt::Conditional& node, const Rect& world, const Subview& view, const EditorContext& ctx);

  /** Branch `scope` at `branch`: its backdrop and the divider along its top edge. */
  void drawScope(const pt::Scope& scope, const Rect& branch, const Subview& view, const EditorContext& ctx);

  /** Header and frame chrome, over the branches; selection outlines belong to the caller. */
  void drawFrame(const pt::Conditional& node, const Rect& frame, const Subview& view, const EditorContext& ctx);

  /** A node-variant entry point: a conditional's body is what `drawNode` paints. */
  void draw(const pt::Conditional& node, const Rect& world, const Subview& view, const EditorContext& ctx);

}  // namespace fluir::editor::draw
