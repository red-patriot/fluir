#include "editor/view/draw/conditional.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace fluir::editor::draw {
  namespace {

    // In grid units; deep enough that the conditional keeps room for a node under its header.
    constexpr Limits<Vec2i> SIZE_LIMITS{.lower = Vec2i{10, 10}, .upper = Vec2i{1000, 1000}};
    constexpr Vec2i DEFAULT_SIZE{30, 40};
    // The branch arrow's side, in grid units.
    constexpr double BRANCH_ARROW_UNITS = 3;
    // A wall port's side, in grid units.
    constexpr double PORT_UNITS = 3;

    // Both tags are the same width, so the arrow sits at one place whichever branch shows.
    static_assert(THEN_TAG.size() == ELSE_TAG.size());

    constexpr int dumb_ceil(double x) {
      // HACK: Do a stupid version of ceil here because MSVC doesn't implement std::ceil as constexpr
      return static_cast<int>((x - static_cast<int>(x)) > 0 ? x + 1 : x);
    }

    // A port's side rounded up to whole grid units, so a port on the grid never pokes past one.
    constexpr int PORT_GRID_UNITS = dumb_ceil(PORT_UNITS);

  }  // namespace

  const pt::BlockPort* portOf(const Ports& conditional, PortRef port) {
    if (port.output) {
      return port.index < conditional.outputs.size() ? &conditional.outputs[port.index] : nullptr;
    }
    if (port.index == 0) {
      return &conditional.condition;
    }
    return port.index <= conditional.inputs.size() ? &conditional.inputs[port.index - 1] : nullptr;
  }

  pt::BlockPort* portOf(Ports& conditional, PortRef port) {
    return const_cast<pt::BlockPort*>(portOf(std::as_const(conditional), port));
  }

  std::vector<PortRef> portRefs(const Ports& conditional) {
    std::vector<PortRef> refs;
    for (std::size_t i = 0; i <= conditional.inputs.size(); ++i) {
      refs.push_back({.output = false, .index = i});
    }
    for (std::size_t i = 0; i < conditional.outputs.size(); ++i) {
      refs.push_back({.output = true, .index = i});
    }
    return refs;
  }

  Color color(const Ports&, const EditorContext::Theme& theme) { return theme.conditionalNodeHeader; }

  Vec2i defaultSize(const Ports&) { return DEFAULT_SIZE; }

  Limits<Vec2i> sizeLimits(const Ports& node) {
    int lowest = node.condition.y;
    for (const pt::BlockPorts* wall : {&node.inputs, &node.outputs}) {
      for (const pt::BlockPort& port : *wall) {
        lowest = std::max(lowest, port.y);
      }
    }
    Limits<Vec2i> limits = SIZE_LIMITS;
    limits.lower.y = std::max(limits.lower.y, lowest + PORT_GRID_UNITS);
    return limits;
  }

  Limits<int> portYLimits(int height, const EditorContext::Layout& layout) {
    return {.lower = static_cast<int>(std::ceil(layout.headerUnits)), .upper = height - PORT_GRID_UNITS};
  }

  std::optional<ResizeAxes> resizePart(const Ports&) { return ResizeAxes::XY; }

  std::vector<FieldLabel> labels(const Ports&, const Rect&, const EditorContext::Layout&) { return {}; }

  Rect branchArrowRect(const Rect& header, const EditorContext::Layout& layout) {
    const double side = BRANCH_ARROW_UNITS * layout.unitPx;
    const Rect tag = splitLabel(header, THEN_TAG, layout).tag;
    return {tag.x + tag.w, header.y + (header.h - side) / 2, side, side};
  }

  Rect portRect(const Ports& node, PortRef port, const Rect& frame, const EditorContext::Layout& layout) {
    const double side = PORT_UNITS * layout.unitPx;
    const double wall = port.output ? frame.x + frame.w : frame.x;
    return {wall - side / 2, frame.y + portOf(node, port)->y * layout.unitPx, side, side};
  }

}  // namespace fluir::editor::draw
