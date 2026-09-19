#include "editor/view/draw/binary.hpp"

#include "compiler/models/operator.hpp"

namespace fluir::editor::draw {

  PortSet anchors(const pt::Binary&, const Rect& r, const EditorContext::Layout&) {
    return {edgeAnchors(r.x, r, 2), edgeAnchors(r.x + r.w, r, 1)};
  }

  Color color(const pt::Binary&, const EditorContext::Theme& theme) { return theme.operatorNode; }

  void draw(const pt::Binary& n, const Rect& world, const Subview& view, const EditorContext& ctx) {
    drawShell(world, color(n, ctx.theme), view, ctx);
    drawTitle(stringify(n.op), world, view, ctx);
    drawPortDots(anchors(n, world, ctx.layout), view, ctx);
  }

}  // namespace fluir::editor::draw
