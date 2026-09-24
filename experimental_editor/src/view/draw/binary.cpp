#include "editor/view/draw/binary.hpp"

#include "compiler/models/operator.hpp"

namespace fluir::editor::draw {

  TerminalSet anchors(const pt::Binary&, const Rect& r, const EditorContext::Layout&) {
    return {edgeAnchors(r.x, r, 2), edgeAnchors(r.x + r.w, r, 1)};
  }

  Color color(const pt::Binary&, const EditorContext::Theme& theme) { return theme.operatorNode; }

  std::vector<FieldLabel> labels(const pt::Binary&, const Rect& world, const EditorContext::Layout&) {
    return {{{Field::Kind::Operator}, world}};
  }

  void draw(const pt::Binary& n, const Rect& world, const Subview& view, const EditorContext& ctx) {
    drawShell(world, color(n, ctx.theme), view, ctx);
    drawTitle(stringify(n.op), labels(n, world, ctx.layout).front().rect, view, ctx);
    drawTerminalDots(anchors(n, world, ctx.layout), view, ctx);
  }

}  // namespace fluir::editor::draw
