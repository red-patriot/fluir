#include "editor/view/draw/unary.hpp"

#include "compiler/models/operator.hpp"

namespace fluir::editor::draw {
  namespace {

    // In grid units. A node's height follows its content, so it is unbounded below.
    constexpr Limits<Vec2i> SIZE_LIMITS{.lower = Vec2i{4, 0}, .upper = Vec2i{1000, 1000}};

  }  // namespace

  TerminalSet anchors(const pt::Unary&, const Rect& r, const EditorContext::Layout&) {
    return {edgeAnchors(r.x, r, 1), edgeAnchors(r.x + r.w, r, 1)};
  }

  Color color(const pt::Unary&, const EditorContext::Theme& theme) { return theme.operatorNode; }

  Limits<Vec2i> sizeLimits(const pt::Unary&) { return SIZE_LIMITS; }

  std::optional<Part> resizePart(const pt::Unary&) { return Part::ResizeX; }

  std::vector<FieldLabel> labels(const pt::Unary&, const Rect& world, const EditorContext::Layout&) {
    return {{{Field::Kind::Operator}, world}};
  }

  void draw(const pt::Unary& n, const Rect& world, const Subview& view, const EditorContext& ctx) {
    drawShell(world, color(n, ctx.theme), view, ctx);
    drawTitle(stringify(n.op), labels(n, world, ctx.layout).front().rect, view, ctx);
    drawTerminalDots(anchors(n, world, ctx.layout), view, ctx);
  }

}  // namespace fluir::editor::draw
