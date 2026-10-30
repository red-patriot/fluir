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

  /** The ports that a conditional node has. */
  struct Ports {
    pt::BlockPort condition;
    pt::BlockPorts inputs;
    pt::BlockPorts outputs;

    friend bool operator==(const Ports&, const Ports&) = default;
  };

  /** A container port: its side, and its index on that side (the condition is input 0). */
  struct PortRef {
    bool output = false;
    std::size_t index = 0;

    friend bool operator==(const PortRef&, const PortRef&) = default;
  };

  /** The port `port` names on `conditional` if it exists, or nullptr. */
  const pt::BlockPort* portOf(const Ports& conditional, PortRef port);

  /** Every port on `conditional`. */
  std::vector<PortRef> portRefs(const Ports& conditional);

  /** The arrow that switches the shown branch, just right of the header's tag. `header` is the header band. */
  Rect branchArrowRect(const Rect& header, const EditorContext::Layout& layout);

  /** Creates a rect for the given `port` */
  Rect portRect(const Ports& node, PortRef port, const Rect& frame, const EditorContext::Layout& layout);

  /** Limits for a port location in grid units, on a conditional `height` grid units tall. */
  Limits<int> portYLimits(int height, const EditorContext::Layout& layout);

  /** The outer side of the conditional ports. */
  TerminalSet anchors(const Ports& node, const Rect& world, const EditorContext::Layout& layout);

  /** The inner side of the conditional ports by inner id. */
  std::unordered_map<fluir::ID, TerminalSet> innerAnchors(const Ports& node,
                                                          const Rect& frame,
                                                          const EditorContext::Layout& layout);

  Color color(const Ports& node, const EditorContext::Theme& theme);

  /** How far the node may be resized in grid units. */
  Limits<Vec2i> sizeLimits(const Ports& node);

  /** The grip that resizes the node, if any. */
  std::optional<ResizeAxes> resizePart(const Ports& node);

  /** None: a conditional has no editable text. */
  std::vector<FieldLabel> labels(const Ports& node, const Rect& world, const EditorContext::Layout& layout);

}  // namespace fluir::editor::draw

#endif
