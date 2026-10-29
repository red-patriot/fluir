#ifndef FLUIR_EDITOR_VIEW_DRAW_CONDITIONAL_HPP
#define FLUIR_EDITOR_VIEW_DRAW_CONDITIONAL_HPP

#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "compiler/models/id.hpp"
#include "editor/core/tree.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/view/draw/draw_utils.hpp"
#include "editor/view/graph_layout.hpp"

namespace fluir::editor::draw {

  inline constexpr std::string_view THEN_TAG = "then";
  inline constexpr std::string_view ELSE_TAG = "else";

  /** The tag of the branch `branchId` names, by its 1-based branch index. */
  std::string_view branchTag(fluir::ID branchId);

  /** The arrow that switches the shown branch, just right of the header's tag. `header` is the header band. */
  Rect branchArrowRect(const Rect& header, const EditorContext::Layout& layout);

  /** Creates a rect for the given `port` */
  Rect portRect(const et::Conditional& node, PortRef port, const Rect& frame, const EditorContext::Layout& layout);

  /** Limits for a port location in grid units. */
  Limits<int> portYLimits(const et::Conditional& node, const EditorContext::Layout& layout);

  /** The outer side of the conditional ports. */
  TerminalSet anchors(const et::Conditional& node, const Rect& world, const EditorContext::Layout& layout);

  /** The inner side of the conditional ports by inner id. */
  std::unordered_map<fluir::ID, TerminalSet> innerAnchors(const et::Conditional& node,
                                                          const Rect& frame,
                                                          const EditorContext::Layout& layout);

  Color color(const et::Conditional& node, const EditorContext::Theme& theme);

  /** How far the node may be resized in grid units. */
  Limits<Vec2i> sizeLimits(const et::Conditional& node);

  /** The grip that resizes the node, if any. */
  std::optional<Part> resizePart(const et::Conditional& node);

  /** None: a conditional has no editable text. */
  std::vector<FieldLabel> labels(const et::Conditional& node, const Rect& world, const EditorContext::Layout& layout);

}  // namespace fluir::editor::draw

#endif
