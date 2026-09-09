#include "editor/actors/node_actors.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include <fmt/format.h>

#include "compiler/models/operator.hpp"
#include "editor/core/graph_geometry.hpp"
#include "editor/core/renderer.hpp"

namespace fluir::editor {
  using namespace ::fluir::literals_types;

  namespace {
    // Mirrors render_graph.cpp's private `renderLiteral`: I8 / U8 widen to int
    // so they print as numbers, BOOL prints true/false, and every other
    // arithmetic type goes straight through fmt.
    std::string renderLiteral(const pt::Literal& value) {
      return std::visit(
        [](auto v) -> std::string {
          using T = std::decay_t<decltype(v)>;
          if constexpr (std::is_same_v<T, bool>) {
            return v ? "true" : "false";
          } else if constexpr (std::is_same_v<T, std::int8_t> || std::is_same_v<T, std::uint8_t>) {
            return fmt::format("{}", static_cast<int>(v));
          } else {
            return fmt::format("{}", v);
          }
        },
        value);
    }

    struct LiteralColor {
      const EditorContext::Theme& theme_;

      Color operator()(const F64&) { return theme_.floatNode; };
      Color operator()(const I8&) { return theme_.sIntNode; };
      Color operator()(const I16&) { return theme_.sIntNode; };
      Color operator()(const I32&) { return theme_.sIntNode; };
      Color operator()(const I64&) { return theme_.sIntNode; };
      Color operator()(const U8&) { return theme_.uIntNode; };
      Color operator()(const U16&) { return theme_.uIntNode; };
      Color operator()(const U32&) { return theme_.uIntNode; };
      Color operator()(const U64&) { return theme_.uIntNode; };
    };

    // Ported from editor/client NodeInOut.tsx `positionFunc`, which returns the
    // CSS-translate percentage `count == 1 ? 0 : 100 * count * (i/(count-1) - 0.5)`
    // — a centred offset. As a fraction of the node's height along [0, 1] that is
    // `i / (count - 1)` for `count > 1`, and the centre (0.5) for a lone port
    // (the client's `count == 1 ? 0` means "no offset from the centre").
    double portFraction(int count, int index) {
      if (count <= 1) {
        return 0.5;
      }
      return static_cast<double>(index) / (static_cast<double>(count) - 1.0);
    }

    // `count` port anchors spread top-to-bottom down an edge at local x == `edgeX`.
    std::vector<Vec2> edgeAnchors(double edgeX, const Rect& nodeRect, int count) {
      std::vector<Vec2> anchors;
      anchors.reserve(static_cast<std::size_t>(count));
      for (int i = 0; i < count; ++i) {
        anchors.push_back(Vec2{edgeX, nodeRect.y + portFraction(count, i) * nodeRect.h});
      }
      return anchors;
    }

    // Draws a dot for each of `anchors` (in `view`-local coords).
    void drawDots(const std::vector<Vec2>& anchors, double portDot, const Subview& view, const Color& color) {
      for (const Vec2& anchor : anchors) {
        view.renderer().fillRect(view.toScreen(dotRect(anchor, portDot)), color);
      }
    }

  }  // namespace

  BinaryActor::BinaryActor(fluir::ID functionId, pt::Binary node, Rect bound) :
    NodeActor(fluir::FullID{functionId, node.id}, bound), node_(node) { }

  void BinaryActor::onClick(Vec2) { lastClickSummary_ = fmt::format("binary {}", stringify(node_.op)); }

  void BinaryActor::drawSelf(const Subview& body, const EditorContext& ctx) const {
    const Rect& nodeRect = bounds();
    body.renderer().fillRect(body.toScreen(nodeRect), ctx.theme.operatorNode);
    body.renderer().drawRect(body.toScreen(nodeRect), ctx.theme.border);

    const Vec2 textPos{nodeRect.x + ctx.layout.textPad, nodeRect.y + ctx.layout.textPad};
    body.renderer().drawText(body.toScreen(textPos), stringify(node_.op), ctx.theme.text);

    drag_.draw(body, ctx, node_.location, nodeRect);

    drawDots(edgeAnchors(nodeRect.x, nodeRect, 2), ctx.layout.portDot, body, ctx.theme.border);
    drawDots(edgeAnchors(nodeRect.x + nodeRect.w, nodeRect, 1), ctx.layout.portDot, body, ctx.theme.border);
  }

  PortSet BinaryActor::ports(const EditorContext& ctx) const {
    const Rect& nodeRect = bounds();
    return {edgeAnchors(nodeRect.x, nodeRect, 2), edgeAnchors(nodeRect.x + nodeRect.w, nodeRect, 1)};
  }

  UnaryActor::UnaryActor(fluir::ID functionId, pt::Unary node, Rect bounds) :
    NodeActor(fluir::FullID{functionId, node.id}, bounds), node_(node) { }

  void UnaryActor::onClick(Vec2) { lastClickSummary_ = fmt::format("unary {}", stringify(node_.op)); }

  void UnaryActor::drawSelf(const Subview& body, const EditorContext& ctx) const {
    const Rect& nodeRect = bounds();
    body.renderer().fillRect(body.toScreen(nodeRect), ctx.theme.operatorNode);
    body.renderer().drawRect(body.toScreen(nodeRect), ctx.theme.border);

    const Vec2 textPos{nodeRect.x + ctx.layout.textPad, nodeRect.y + ctx.layout.textPad};
    body.renderer().drawText(body.toScreen(textPos), stringify(node_.op), ctx.theme.text);

    drag_.draw(body, ctx, node_.location, nodeRect);

    drawDots(edgeAnchors(nodeRect.x, nodeRect, 1), ctx.layout.portDot, body, ctx.theme.border);
    drawDots(edgeAnchors(nodeRect.x + nodeRect.w, nodeRect, 1), ctx.layout.portDot, body, ctx.theme.border);
  }

  PortSet UnaryActor::ports(const EditorContext& ctx) const {
    const Rect& nodeRect = bounds();
    return {edgeAnchors(nodeRect.x, nodeRect, 1), edgeAnchors(nodeRect.x + nodeRect.w, nodeRect, 1)};
  }

  ConstantActor::ConstantActor(fluir::ID functionId, pt::Constant node, Rect bounds) :
    NodeActor(fluir::FullID{functionId, node.id}, bounds), node_(node) { }

  void ConstantActor::onClick(Vec2) { lastClickSummary_ = fmt::format("constant {}", renderLiteral(node_.value)); }

  void ConstantActor::drawSelf(const Subview& body, const EditorContext& ctx) const {
    const Rect& nodeRect = bounds();
    body.renderer().fillRect(body.toScreen(nodeRect), std::visit(LiteralColor{ctx.theme}, node_.value));
    body.renderer().drawRect(body.toScreen(nodeRect), ctx.theme.border);

    const Vec2 textPos{nodeRect.x + ctx.layout.textPad, nodeRect.y + ctx.layout.textPad};
    body.renderer().drawText(body.toScreen(textPos), renderLiteral(node_.value), ctx.theme.text);

    drag_.draw(body, ctx, node_.location, nodeRect);

    drawDots(edgeAnchors(nodeRect.x + nodeRect.w, nodeRect, 1), ctx.layout.portDot, body, ctx.theme.border);
  }

  PortSet ConstantActor::ports(const EditorContext& ctx) const {
    const Rect& nodeRect = bounds();
    return {{}, edgeAnchors(nodeRect.x + nodeRect.w, nodeRect, 1)};
  }

  CallActor::CallActor(fluir::ID functionId, pt::Call node, Rect bounds) :
    NodeActor(fluir::FullID{functionId, node.id}, bounds), node_(node) { }

  void CallActor::onClick(Vec2) { lastClickSummary_ = fmt::format("call {}", node_.target); }

  namespace {

    // Sorted-by-index argument pointers, shared by CallActor::draw and CallActor::ports.
    std::vector<const pt::Call::Argument*> sortedArgs(const pt::Call& call) {
      std::vector<const pt::Call::Argument*> args;
      args.reserve(call.arguments.size());
      for (const auto& arg : call.arguments) {
        args.push_back(&arg);
      }
      std::sort(args.begin(), args.end(), [](const pt::Call::Argument* a, const pt::Call::Argument* b) {
        return a->index < b->index;
      });
      return args;
    }

    // One row per argument, stacked below the header row (railStep() tall),
    // matching CallNode.tsx's column of CallArgumentNode rows; each row's
    // input handle sits at the row's vertical centre.
    std::vector<Vec2> callArgumentAnchors(const Rect& nodeRect,
                                          const std::vector<const pt::Call::Argument*>& args,
                                          const EditorContext::Layout& layout) {
      std::vector<Vec2> anchors;
      anchors.reserve(args.size());
      for (std::size_t k = 0; k < args.size(); ++k) {
        const double rowTop = nodeRect.y + (static_cast<double>(k) + 1.0) * layout.railStep();
        anchors.push_back(Vec2{nodeRect.x, rowTop + layout.railStep() * 0.5});
      }
      return anchors;
    }

  }  // namespace

  void CallActor::drawSelf(const Subview& body, const EditorContext& ctx) const {
    const Rect& nodeRect = bounds();
    body.renderer().fillRect(body.toScreen(nodeRect), ctx.theme.callNode);
    body.renderer().drawRect(body.toScreen(nodeRect), ctx.theme.border);

    const Vec2 textPos{nodeRect.x + ctx.layout.textPad, nodeRect.y + ctx.layout.textPad};
    body.renderer().drawText(body.toScreen(textPos), node_.target, ctx.theme.text);

    const std::vector<const pt::Call::Argument*> args = sortedArgs(node_);
    const std::vector<Vec2> argAnchors = callArgumentAnchors(nodeRect, args, ctx.layout);
    for (std::size_t k = 0; k < args.size(); ++k) {
      const double rowTop = nodeRect.y + (static_cast<double>(k) + 1.0) * ctx.layout.railStep();
      body.renderer().drawText(body.toScreen(Vec2{nodeRect.x + ctx.layout.textPad, rowTop + ctx.layout.textPad}),
                               args[k]->name,
                               ctx.theme.text);
    }

    drag_.draw(body, ctx, node_.location, nodeRect);

    drawDots(argAnchors, ctx.layout.portDot, body, ctx.theme.border);

    if (node_._return.has_value()) {
      drawDots(edgeAnchors(nodeRect.x + nodeRect.w, nodeRect, 1), ctx.layout.portDot, body, ctx.theme.border);
    }
  }

  PortSet CallActor::ports(const EditorContext& ctx) const {
    const Rect& nodeRect = bounds();
    PortSet result;
    result.inputs = callArgumentAnchors(nodeRect, sortedArgs(node_), ctx.layout);
    if (node_._return.has_value()) {
      result.outputs = edgeAnchors(nodeRect.x + nodeRect.w, nodeRect, 1);
    }
    return result;
  }

}  // namespace fluir::editor
