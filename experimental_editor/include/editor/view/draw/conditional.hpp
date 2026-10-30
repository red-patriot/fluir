#ifndef FLUIR_EDITOR_VIEW_DRAW_CONDITIONAL_HPP
#define FLUIR_EDITOR_VIEW_DRAW_CONDITIONAL_HPP

#include <cstddef>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor::draw {

  inline constexpr std::string_view THEN_TAG = "then";
  inline constexpr std::string_view ELSE_TAG = "else";

  /** A container port: its side, and its index on that side (the condition is input 0). */
  struct PortRef {
    bool output = false;
    std::size_t index = 0;

    friend bool operator==(const PortRef&, const PortRef&) = default;
  };

  /** The port `port` names on `conditional` if it exists, or nullptr. */
  const pt::BlockPort* portOf(const pt::Conditional& conditional, PortRef port);

  /** Every port on `conditional`. */
  std::vector<PortRef> portRefs(const pt::Conditional& conditional);

  /** The arrow that switches the shown branch, just right of the header's tag. `header` is the header band. */
  Rect branchArrowRect(const Rect& header, const EditorContext::Layout& layout);

  /** Creates a rect for the given `port` */
  Rect portRect(const pt::Conditional& node, PortRef port, const Rect& frame, const EditorContext::Layout& layout);

  /** Limits for a port location in grid units. */
  Limits<int> portYLimits(const pt::Conditional& node, const EditorContext::Layout& layout);

  /** The outer side of the conditional ports. */
  TerminalSet anchors(const pt::Conditional& node, const Rect& world, const EditorContext::Layout& layout);

  /** The inner side of the conditional ports by inner id. */
  std::unordered_map<fluir::ID, TerminalSet> innerAnchors(const pt::Conditional& node,
                                                          const Rect& frame,
                                                          const EditorContext::Layout& layout);

  Color color(const pt::Conditional& node, const EditorContext::Theme& theme);

  /** How far the node may be resized in grid units. */
  Limits<Vec2i> sizeLimits(const pt::Conditional& node);

  /** The grip that resizes the node, if any. */
  std::optional<ResizeAxes> resizePart(const pt::Conditional& node);

  /** None: a conditional has no editable text. */
  std::vector<FieldLabel> labels(const pt::Conditional& node, const Rect& world, const EditorContext::Layout& layout);

}  // namespace fluir::editor::draw

#endif
