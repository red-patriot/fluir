#include "editor/view/draw/constant.hpp"

#include <algorithm>
#include <variant>

#include "editor/core/literal_text.hpp"

namespace fluir::editor::draw {
  namespace {
    using namespace ::fluir::literals_types;

    // In grid units. A node's height follows its content, so it is unbounded below.
    constexpr Limits<Vec2i> SIZE_LIMITS{.lower = Vec2i{4, 0}, .upper = Vec2i{1000, 1000}};

    // Gap between the bool square and the node's edges, in world px.
    constexpr double TOGGLE_PAD = 4.0;

    struct LiteralColor {
      const EditorContext::Theme& theme;
      Color operator()(const F64&) const { return theme.floatNode; }
      Color operator()(const I8&) const { return theme.sIntNode; }
      Color operator()(const I16&) const { return theme.sIntNode; }
      Color operator()(const I32&) const { return theme.sIntNode; }
      Color operator()(const I64&) const { return theme.sIntNode; }
      Color operator()(const U8&) const { return theme.uIntNode; }
      Color operator()(const U16&) const { return theme.uIntNode; }
      Color operator()(const U32&) const { return theme.uIntNode; }
      Color operator()(const U64&) const { return theme.uIntNode; }
      Color operator()(const BOOL&) const { return theme.boolNode; }
    };

  }  // namespace

  Rect boolToggleRect(const Rect& world, const EditorContext::Layout& layout) {
    const double side =
      std::max(0.0, std::min(world.h - 1 * TOGGLE_PAD, moveGrip(world, layout.unitPx).x - world.x - 1 * TOGGLE_PAD));
    return {world.x + TOGGLE_PAD, world.y + (world.h - side) / 2, side, side};
  }

  std::vector<FieldLabel> labels(const pt::Constant& n, const Rect& world, const EditorContext::Layout& layout) {
    if (std::holds_alternative<BOOL>(n.value)) {
      return {{{Field::Kind::Bool}, boolToggleRect(world, layout)}};
    }
    return {{{Field::Kind::Literal}, splitLabel(world, literalTypeName(n.value), layout).text}};
  }

  Limits<Vec2i> sizeLimits(const pt::Constant&) { return SIZE_LIMITS; }

  // A bool draws a fixed-size toggle square, so there is nothing to widen.
  std::optional<ResizeAxes> resizePart(const pt::Constant& c) {
    return std::holds_alternative<BOOL>(c.value) ? std::nullopt : std::optional{ResizeAxes::X};
  }

  TerminalSet anchors(const pt::Constant&, const Rect& r, const EditorContext::Layout&) {
    return {{}, edgeAnchors(r.x + r.w, r, 1)};
  }

  Color color(const pt::Constant& c, const EditorContext::Theme& theme) {
    return std::visit(LiteralColor{theme}, c.value);
  }

}  // namespace fluir::editor::draw
