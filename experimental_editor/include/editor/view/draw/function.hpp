#pragma once

#include <string_view>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::draw {

  /** The small tag in a function's header label. */
  inline constexpr std::string_view FN_TAG = "fn";

  /** A rail's terminal: the return's input at left-mid, a parameter's output at right-mid. */
  TerminalSet anchors(const pt::FunctionDecl& fn, fluir::ID railId, const Rect& rail);

  Color color(const pt::FunctionDecl& fn, const EditorContext::Theme& theme);

  // A function paints in parts at different depths, so each part has its own draw.

  /** The body background, under the function's nodes. */
  void drawBody(const pt::FunctionDecl& fn, const Rect& world, const Subview& view, const EditorContext& ctx);

  /** Rail `railId` at `rail`: shell, type and name label, terminal dot. */
  void drawRail(
    const pt::FunctionDecl& fn, fluir::ID railId, const Rect& rail, const Subview& view, const EditorContext& ctx);

  /** Header and frame chrome, over the function's nodes; selection outlines belong to the caller. */
  void drawFrame(const pt::FunctionDecl& fn, const Rect& frame, const Subview& view, const EditorContext& ctx);

}  // namespace fluir::editor::draw
