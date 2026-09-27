#include "editor/view/draw/conditional.hpp"

#include <algorithm>
#include <cmath>

#include "editor/core/renderer.hpp"
#include "editor/core/tree_path.hpp"

namespace fluir::editor::draw {
  namespace {

    // In grid units; deep enough that the conditional keeps room for a node under its header.
    constexpr Limits<Vec2i> SIZE_LIMITS{.lower = Vec2i{10, 10}, .upper = Vec2i{1000, 1000}};
    // The branch arrow's side, in grid units.
    constexpr double BRANCH_ARROW_UNITS = 3;
    // A wall port's side, in grid units.
    constexpr double PORT_UNITS = 3;

    // Both tags are the same width, so the arrow sits at one place whichever branch shows.
    static_assert(THEN_TAG.size() == ELSE_TAG.size());

    // A port's outer and inner anchors: the midpoints of its left and right edges.
    struct PortAnchors {
      Vec2 outer;
      Vec2 inner;
    };

    // A port's side rounded up to whole grid units, so a port on the grid never pokes past one.
    constexpr int PORT_GRID_UNITS = static_cast<int>(std::ceil(PORT_UNITS));

    PortAnchors portAnchors(const Rect& port) {
      const double midY = port.y + port.h / 2;
      return {Vec2{port.x, midY}, Vec2{port.x + port.w, midY}};
    }

  }  // namespace

  TerminalSet anchors(const et::Conditional& node, const Rect& world, const EditorContext::Layout& layout) {
    return {.inputs = {portAnchors(conditionPortRect(node, world, layout)).outer}};
  }

  std::unordered_map<fluir::ID, TerminalSet> innerAnchors(const et::Conditional& node,
                                                          const Rect& frame,
                                                          const EditorContext::Layout& layout) {
    if (node.condition.innerId == INVALID_ID) {
      return {};
    }
    return {{node.condition.innerId, {.outputs = {portAnchors(conditionPortRect(node, frame, layout)).inner}}}};
  }

  Color color(const et::Conditional&, const EditorContext::Theme& theme) { return theme.conditionalNodeHeader; }

  Limits<Vec2i> sizeLimits(const et::Conditional& node) {
    int lowest = node.condition.y;
    for (const et::BlockPorts* wall : {&node.inputs, &node.outputs}) {
      for (const et::BlockPort& port : *wall) {
        lowest = std::max(lowest, port.y);
      }
    }
    Limits<Vec2i> limits = SIZE_LIMITS;
    limits.lower.y = std::max(limits.lower.y, lowest + PORT_GRID_UNITS);
    return limits;
  }

  Limits<int> portYLimits(const et::Conditional& node, const EditorContext::Layout& layout) {
    return {.lower = static_cast<int>(std::ceil(layout.headerUnits)), .upper = node.location.height - PORT_GRID_UNITS};
  }

  std::optional<Part> resizePart(const et::Conditional&) { return Part::ResizeXY; }

  std::vector<FieldLabel> labels(const et::Conditional&, const Rect&, const EditorContext::Layout&) { return {}; }

  void drawBody(const et::Conditional&, const Rect& world, const Subview& view, const EditorContext& ctx) {
    view.renderer().fillRect(view.toScreen(world), ctx.theme.background);
  }

  Rect branchArrowRect(const Rect& header, const EditorContext::Layout& layout) {
    const double side = BRANCH_ARROW_UNITS * layout.unitPx;
    const Rect tag = splitLabel(header, THEN_TAG, layout).tag;
    return {tag.x + tag.w, header.y + (header.h - side) / 2, side, side};
  }

  Rect conditionPortRect(const et::Conditional& node, const Rect& frame, const EditorContext::Layout& layout) {
    const double side = PORT_UNITS * layout.unitPx;
    return {frame.x - side / 2, frame.y + node.condition.y * layout.unitPx, side, side};
  }

  void drawPort(const Rect& world, const Subview& view, const EditorContext& ctx) {
    view.renderer().fillRect(view.toScreen(world), ctx.theme.boolNode);
    view.renderer().drawRect(view.toScreen(world), ctx.theme.border);
  }

  std::string_view branchTag(fluir::ID branchId) { return branchId == ELSE_BRANCH_ID ? ELSE_TAG : THEN_TAG; }

  void drawFrame(const et::Conditional& node, const Rect& frame, const Subview& view, const EditorContext& ctx) {
    const Rect header{frame.x, frame.y, frame.w, ctx.layout.headerH()};
    view.renderer().fillRect(view.toScreen(header), color(node, ctx.theme));
    view.renderer().drawRect(view.toScreen(frame), ctx.theme.border);
    drawSplitLabel(view, header, branchTag(node.annotation.shownBranch), ctx);
    drawImage(assets::branchArrowIcon(), branchArrowRect(header, ctx.layout), view, ctx.theme.text);
  }

  void draw(const et::Conditional& node, const Rect& world, const Subview& view, const EditorContext& ctx) {
    drawBody(node, world, view, ctx);
  }

}  // namespace fluir::editor::draw
