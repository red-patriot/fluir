#pragma once

#include <string_view>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::draw {

  inline constexpr std::string_view THEN_TAG = "then";
  inline constexpr std::string_view ELSE_TAG = "else";

  /** Which of `conditional`'s two branches `scope` is. */
  std::string_view branchTag(const pt::Conditional& conditional, const pt::Scope& scope);

  /** None yet: a conditional wires through its scope terminals, which is Phase 2. */
  TerminalSet anchors(const pt::Conditional& node, const Rect& world, const EditorContext::Layout& layout);

  Color color(const pt::Conditional& node, const EditorContext::Theme& theme);

  // A conditional paints in parts at different depths, so each part has its own draw.

  /** The body background, under the conditional's branches and their nodes. */
  void drawBody(const pt::Conditional& node, const Rect& world, const Subview& view, const EditorContext& ctx);

  /** Branch `scope` at `branch`: the divider along its top edge and its tagged header. */
  void drawScope(
    const pt::Scope& scope, std::string_view tag, const Rect& branch, const Subview& view, const EditorContext& ctx);

  /** The border around both branches. */
  void drawFrame(const pt::Conditional& node, const Rect& frame, const Subview& view, const EditorContext& ctx);

  /** A node-variant entry point. */
  void draw(const pt::Conditional& node, const Rect& world, const Subview& view, const EditorContext& ctx);

}  // namespace fluir::editor::draw
