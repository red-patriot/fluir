#include "editor/view/node_view.hpp"

#include <algorithm>
#include <cstddef>
#include <variant>

#include "compiler/models/operator.hpp"
#include "editor/core/graph_geometry.hpp"
#include "editor/core/literal_text.hpp"
#include "editor/core/renderer.hpp"

// Each node kind is one overload of `anchors`, `color` and `label`. A new kind adds its overloads here.

namespace fluir::editor {
  namespace {
    using namespace ::fluir::literals_types;

    // A lone port sits at the centre; more spread top-to-bottom down the edge.
    double portFraction(int count, int index) {
      return count <= 1 ? 0.5 : static_cast<double>(index) / (static_cast<double>(count) - 1.0);
    }

    std::vector<Vec2> edgeAnchors(double edgeX, const Rect& rect, int count) {
      std::vector<Vec2> out;
      for (int i = 0; i < count; ++i) {
        out.push_back(Vec2{edgeX, rect.y + portFraction(count, i) * rect.h});
      }
      return out;
    }

    std::vector<const pt::Call::Argument*> sortedArgs(const pt::Call& call) {
      std::vector<const pt::Call::Argument*> args;
      for (const auto& arg : call.arguments) {
        args.push_back(&arg);
      }
      std::ranges::sort(args, {}, &pt::Call::Argument::index);
      return args;
    }

    // One row per argument below the header row, each input at its row's centre.
    double argRowTop(const Rect& rect, std::size_t row, const EditorContext::Layout& layout) {
      return rect.y + (static_cast<double>(row) + 1.0) * layout.railStep();
    }

    PortSet anchors(const pt::Binary&, const Rect& r, const EditorContext::Layout&) {
      return {edgeAnchors(r.x, r, 2), edgeAnchors(r.x + r.w, r, 1)};
    }
    PortSet anchors(const pt::Comment&, const Rect&, const EditorContext::Layout&) { return {}; };
    PortSet anchors(const pt::Unary&, const Rect& r, const EditorContext::Layout&) {
      return {edgeAnchors(r.x, r, 1), edgeAnchors(r.x + r.w, r, 1)};
    }
    PortSet anchors(const pt::Constant&, const Rect& r, const EditorContext::Layout&) {
      return {{}, edgeAnchors(r.x + r.w, r, 1)};
    }
    PortSet anchors(const pt::Call& call, const Rect& r, const EditorContext::Layout& layout) {
      PortSet out;
      const std::size_t count = call.arguments.size();
      for (std::size_t row = 0; row < count; ++row) {
        out.inputs.push_back(Vec2{r.x, argRowTop(r, row, layout) + layout.railStep() * 0.5});
      }
      if (call._return) {
        out.outputs = edgeAnchors(r.x + r.w, r, 1);
      }
      return out;
    }

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

    Color color(const pt::Binary&, const EditorContext::Theme& theme) { return theme.operatorNode; }
    Color color(const pt::Unary&, const EditorContext::Theme& theme) { return theme.operatorNode; }
    Color color(const pt::Constant& c, const EditorContext::Theme& theme) {
      return std::visit(LiteralColor{theme}, c.value);
    }
    Color color(const pt::Call&, const EditorContext::Theme& theme) { return theme.callNode; }
    Color color(const pt::Comment&, const EditorContext::Theme& theme) { return theme.commentNode; }

    std::string label(const pt::Binary& n) { return std::string{stringify(n.op)}; }
    std::string label(const pt::Unary& n) { return std::string{stringify(n.op)}; }
    std::string label(const pt::Constant& n) { return renderLiteral(n.value); }
    std::string label(const pt::Call& n) { return n.target; }
    std::string label(const pt::Comment&) { return "//"; }

    // Call argument names, one per row; other kinds carry nothing extra.
    void drawExtras(const auto&, const Rect&, const Subview&, const EditorContext&) { }
    void drawExtras(const pt::Call& call, const Rect& r, const Subview& view, const EditorContext& ctx) {
      const std::vector<const pt::Call::Argument*> args = sortedArgs(call);
      for (std::size_t row = 0; row < args.size(); ++row) {
        const Vec2 pos{r.x + ctx.layout.textPad, argRowTop(r, row, ctx.layout) + ctx.layout.textPad};
        view.renderer().drawText(view.toScreen(pos), args[row]->name, ctx.theme.text);
      }
    }

    void drawKind(const auto& n, const Rect& world, const Subview& view, const EditorContext& ctx) {
      Renderer& r = view.renderer();
      r.fillRect(view.toScreen(world), color(n, ctx.theme));
      r.drawRect(view.toScreen(world), ctx.theme.border);
      const Vec2 textPos{world.x + ctx.layout.textPad, world.y + ctx.layout.textPad};
      r.drawText(view.toScreen(textPos), label(n), ctx.theme.text);
      drawExtras(n, world, view, ctx);
      const PortSet anchorsOf = anchors(n, world, ctx.layout);
      for (const auto* side : {&anchorsOf.inputs, &anchorsOf.outputs}) {
        for (const Vec2& anchor : *side) {
          r.fillRect(view.toScreen(dotRect(anchor, ctx.layout.portDot)), ctx.theme.border);
        }
      }
    }

  }  // namespace

  PortSet ports(const pt::Node& node, Rect world, const EditorContext::Layout& layout) {
    return std::visit([&](const auto& n) { return anchors(n, world, layout); }, node);
  }

  Color nodeColor(const pt::Node& node, const EditorContext::Theme& theme) {
    return std::visit([&](const auto& n) { return color(n, theme); }, node);
  }

  void drawNode(const pt::Node& node, Rect world, const Subview& view, const EditorContext& ctx) {
    std::visit([&](const auto& n) { drawKind(n, world, view, ctx); }, node);
  }

  void drawComment(const pt::Comment& comment, Rect world, const Subview& view, const EditorContext& ctx) {
    drawKind(comment, world, view, ctx);
  }

}  // namespace fluir::editor
