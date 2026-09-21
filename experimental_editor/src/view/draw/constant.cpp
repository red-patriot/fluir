#include "editor/view/draw/constant.hpp"

#include <algorithm>
#include <variant>

#include "editor/assets/images.hpp"
#include "editor/core/literal_text.hpp"
#include "editor/core/renderer.hpp"
#include "editor/view/graph_layout.hpp"

namespace fluir::editor::draw {
  namespace {
    using namespace ::fluir::literals_types;

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

  TerminalSet anchors(const pt::Constant&, const Rect& r, const EditorContext::Layout&) {
    return {{}, edgeAnchors(r.x + r.w, r, 1)};
  }

  Color color(const pt::Constant& c, const EditorContext::Theme& theme) {
    return std::visit(LiteralColor{theme}, c.value);
  }

  void draw(const pt::Constant& n, const Rect& world, const Subview& view, const EditorContext& ctx) {
    drawShell(world, color(n, ctx.theme), view, ctx);
    // A bool shows its value as a checkbox instead of a label: outlined when false, filled when true.
    if (const auto* flag = std::get_if<BOOL>(&n.value)) {
      const Rect square = boolToggleRect(world, ctx.layout);
      view.renderer().drawIcon(
        view.toScreen(square), *flag ? assets::trueIcon() : assets::falseIcon(), ctx.theme.border);

    } else {
      drawSplitLabel(view, world, literalTypeName(n.value), renderLiteral(n.value), ctx);
    }
    drawTerminalDots(anchors(n, world, ctx.layout), view, ctx);
  }

}  // namespace fluir::editor::draw
