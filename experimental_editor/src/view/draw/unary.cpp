#include "editor/view/draw/unary.hpp"

#include "compiler/models/operator.hpp"

namespace fluir::editor::draw {

  TerminalSet anchors(const pt::Unary&, const Rect& r, const EditorContext::Layout&) {
    return {edgeAnchors(r.x, r, 1), edgeAnchors(r.x + r.w, r, 1)};
  }

  Color color(const pt::Unary&, const EditorContext::Theme& theme) { return theme.operatorNode; }

  void draw(const pt::Unary& n, const Rect& world, const Subview& view, const EditorContext& ctx) {
    drawShell(world, color(n, ctx.theme), view, ctx);
    drawTitle(stringify(n.op), world, view, ctx);
    drawTerminalDots(anchors(n, world, ctx.layout), view, ctx);
  }

}  // namespace fluir::editor::draw
