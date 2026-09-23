#pragma once

#include <string_view>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::draw {

  inline constexpr std::string_view THEN_TAG = "then";
  inline constexpr std::string_view ELSE_TAG = "else";

  /** The tag of the branch `branchId` names, by its 1-based branch index. */
  std::string_view branchTag(fluir::ID branchId);

  /** None yet: a conditional wires through its block terminals, which is Phase 2. */
  TerminalSet anchors(const pt::Conditional& node, const Rect& world, const EditorContext::Layout& layout);

  Color color(const pt::Conditional& node, const EditorContext::Theme& theme);

  // A conditional paints in parts at different depths, so each part has its own draw.

  /** The body background, under the conditional's branches and their nodes. */
  void drawBody(const pt::Conditional& node, const Rect& world, const Subview& view, const EditorContext& ctx);

  /** The header band over the visible branch, its tag, and the border around the frame. */
  void drawFrame(const pt::Conditional& node, const Rect& frame, const Subview& view, const EditorContext& ctx);

  /** A node-variant entry point. */
  void draw(const pt::Conditional& node, const Rect& world, const Subview& view, const EditorContext& ctx);

}  // namespace fluir::editor::draw
