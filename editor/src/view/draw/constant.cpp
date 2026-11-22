#include "editor/view/draw/constant.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <variant>

#include "editor/core/literal_text.hpp"

namespace fluir::editor::draw {
  namespace {
    // In grid units. A node's height follows its content, so it is unbounded below.
    constexpr Limits<Vec2i> SIZE_LIMITS{.lower = Vec2i{4, 0}, .upper = Vec2i{1000, 1000}};
    constexpr Vec2i NUMBER_DEFAULT_SIZE{12, 5};
    constexpr Vec2i BOOL_DEFAULT_SIZE{8, 5};
    constexpr Vec2i STR_DEFAULT_SIZE{15, 15};

    // Gap between the bool square and the node's edges, in world px.
    constexpr double TOGGLE_PAD = 4.0;

    Color literalColor(const pt::Constant& c, const EditorContext::Theme& theme) {
      switch (c.typeTag) {
        case pt::Constant::F64:
          return theme.floatNode;
        case pt::Constant::I8:
        case pt::Constant::I16:
        case pt::Constant::I32:
        case pt::Constant::I64:
          return theme.sIntNode;
        case pt::Constant::U8:
        case pt::Constant::U16:
        case pt::Constant::U32:
        case pt::Constant::U64:
          return theme.uIntNode;
        case pt::Constant::BOOL:
          return theme.boolNode;
        case pt::Constant::STR:
          return theme.strNode;
      }
      std::unreachable();
    }
  }  // namespace

  Rect boolToggleRect(const Rect& world, const EditorContext::Layout& layout) {
    const double side =
      std::max(0.0, std::min(world.h - 1 * TOGGLE_PAD, moveGrip(world, layout.unitPx).x - world.x - 1 * TOGGLE_PAD));
    return {world.x + TOGGLE_PAD, world.y + (world.h - side) / 2, side, side};
  }

  std::vector<FieldLabel> labels(const pt::Constant& n, const Rect& world, const EditorContext::Layout& layout) {
    if (n.typeTag == pt::Constant::BOOL) {
      return {{{Field::Kind::Bool}, boolToggleRect(world, layout)}};
    }
    std::string name{literalTypeName(n)};
    return {{{Field::Kind::Literal}, splitLabel(world, name, layout).text}};
  }

  Limits<Vec2i> sizeLimits(const pt::Constant&) { return SIZE_LIMITS; }

  Vec2i defaultSize(const pt::Constant& c) {
    switch (c.typeTag) {
      case pt::Constant::STR:
        return STR_DEFAULT_SIZE;
      case pt::Constant::BOOL:
        return BOOL_DEFAULT_SIZE;
      default:
        return NUMBER_DEFAULT_SIZE;
    }
  }

  // A bool draws a fixed-size toggle square, so there is nothing to widen.
  std::optional<ResizeAxes> resizePart(const pt::Constant& c) {
    switch (c.typeTag) {
      case pt::Constant::BOOL:
        return std::nullopt;
      case pt::Constant::STR:
        return ResizeAxes::XY;
      default:
        return ResizeAxes::X;
    }
  }

  TerminalSet anchors(const pt::Constant&, const Rect& r, const EditorContext::Layout&) {
    return {{}, edgeAnchors(r.x + r.w, r, 1)};
  }

  Color color(const pt::Constant& c, const EditorContext::Theme& theme) { return literalColor(c, theme); }
}  // namespace fluir::editor::draw
