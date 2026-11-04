#ifndef FLUIR_EDITOR_VIEW_DRAW_FUNCTION_HPP
#define FLUIR_EDITOR_VIEW_DRAW_FUNCTION_HPP

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::draw {

  /** A function signature. */
  struct Signature {
    std::string name;
    std::optional<pt::FunctionInputBlock> input;
    std::optional<pt::FunctionOutputBlock> output;

    friend bool operator==(const Signature&, const Signature&) = default;
  };

  /** The small tag in a function's header label. */
  inline constexpr std::string_view FN_TAG = "fn";

  /** A parameter's or the return's terminal. */
  TerminalSet anchors(const Signature& fn, fluir::ID id, const Rect& rect);

  /** The rect of the parameter or return `id`. */
  Rect paramOrReturnRect(const Signature& fn, fluir::ID id, const Rect& frame, const EditorContext::Layout& layout);

  Color color(const Signature& fn, const EditorContext::Theme& theme);

  /** How far the function may be resized, in grid units. */
  Limits<Vec2i> sizeLimits(const Signature& fn);

  /** The size a new function starts at, in grid units. */
  Vec2i defaultSize(const Signature& node);

  /** The name, after the tag in the frame's header band. */
  std::vector<FieldLabel> labels(const Signature& fn, const Rect& frame, const EditorContext::Layout& layout);

  /** The label of a function parameter or return */
  std::vector<FieldLabel> labels(const Signature& fn,
                                 fluir::ID id,
                                 const Rect& rect,
                                 const EditorContext::Layout& layout);

}  // namespace fluir::editor::draw

#endif
