#ifndef FLUIR_EDITOR_VIEW_DRAW_FUNCTION_HPP
#define FLUIR_EDITOR_VIEW_DRAW_FUNCTION_HPP

#include <string_view>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::draw {

  /** The small tag in a function's header label. */
  inline constexpr std::string_view FN_TAG = "fn";

  /** A rail's terminal: the return's input at left-mid, a parameter's output at right-mid. */
  TerminalSet anchors(const pt::FunctionDecl& fn, fluir::ID railId, const Rect& rail);

  /** Rail `railId`'s rect: parameters stack down the body's left edge, the return sits at its top right. Empty for
   *  an id that is no rail of `fn`. */
  Rect railRect(const pt::FunctionDecl& fn, fluir::ID railId, const Rect& frame, const EditorContext::Layout& layout);

  Color color(const pt::FunctionDecl& fn, const EditorContext::Theme& theme);

  /** How far the function may be resized, in grid units. */
  Limits<Vec2i> sizeLimits(const pt::FunctionDecl& fn);

  /** The name, after the tag in the frame's header band. */
  std::vector<FieldLabel> labels(const pt::FunctionDecl& fn, const Rect& frame, const EditorContext::Layout& layout);

  /** A rail's type tag, then a parameter's name after it; the return rail has no name. */
  std::vector<FieldLabel> labels(const pt::FunctionDecl& fn,
                                 fluir::ID railId,
                                 const Rect& rail,
                                 const EditorContext::Layout& layout);

}  // namespace fluir::editor::draw

#endif
