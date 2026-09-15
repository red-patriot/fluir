#include "editor/view/draw/constant.hpp"

#include <variant>

#include "editor/core/literal_text.hpp"

namespace fluir::editor::draw {
  namespace {
    using namespace ::fluir::literals_types;

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
    };

  }  // namespace

  PortSet anchors(const pt::Constant&, const Rect& r, const EditorContext::Layout&) {
    return {{}, edgeAnchors(r.x + r.w, r, 1)};
  }

  Color color(const pt::Constant& c, const EditorContext::Theme& theme) {
    return std::visit(LiteralColor{theme}, c.value);
  }

  void draw(const pt::Constant& n, const Rect& world, const Subview& view, const EditorContext& ctx) {
    drawShell(world, color(n, ctx.theme), view, ctx);
    drawSplitLabel(view, world, literalTypeName(n.value), renderLiteral(n.value), ctx);
    drawPortDots(anchors(n, world, ctx.layout), view, ctx);
  }

}  // namespace fluir::editor::draw
