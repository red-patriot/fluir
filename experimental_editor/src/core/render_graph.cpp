#include "editor/core/render_graph.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include <fmt/format.h>

#include "compiler/models/id.hpp"
#include "compiler/models/operator.hpp"

namespace fluir::editor {

  namespace {

    // Text inset from a rect's top-left corner (world px).
    constexpr double kTextPad = 4.0;
    // Side length of a port dot (world px).
    constexpr double kPortDot = 6.0;

    // Client constants ported from editor/client: FUNC_HEADER_HEIGHT == 5 and
    // PARAM_BLOCK_HEIGHT == 5 logical units; RETURN_NODE_WIDTH == 5 logical.
    // World px = logical * UNIT_PX (== ZOOM_SCALAR == 5).
    constexpr double kHeaderH = 5.0 * UNIT_PX;   // 25
    constexpr double kRailStep = 5.0 * UNIT_PX;  // 25 — row height and y-step
    constexpr double kParamW = 15.0 * UNIT_PX;   // 75
    constexpr double kReturnInsetLogical = 5.0;  // rail sits at logical x = width - 5

    Rect dotRect(Vec2 anchor) { return {anchor.x - kPortDot * 0.5, anchor.y - kPortDot * 0.5, kPortDot, kPortDot}; }

    // Chrome (frame / header / name / rails) draws with no clip of its own, so it
    // uses the bare viewport transform rather than a Subview (which always
    // clips). `world == origin + local`.
    Vec2 toScreen(const Viewport& vp, Vec2 world) { return vp.worldToScreen(world); }

    Rect toScreen(const Viewport& vp, Rect world) {
      const Vec2 tl = vp.worldToScreen(world.topLeft());
      return {tl.x, tl.y, world.w * vp.scale, world.h * vp.scale};
    }

    // Shift a local-space rect by the function's world origin.
    Rect atOrigin(Vec2 origin, Rect local) { return {origin.x + local.x, origin.y + local.y, local.w, local.h}; }

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

    // Reproduces compiler/src/debug/parse_tree_printer.cpp literal rendering: I8 /
    // U8 widen to int so they print as numbers, BOOL prints true/false, and every
    // other arithmetic type goes straight through fmt.
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

    // Deterministic order: (location.z, id).
    std::vector<const pt::FunctionDecl*> sortedFunctions(const pt::ParseTree& tree) {
      std::vector<const pt::FunctionDecl*> funcs;
      funcs.reserve(tree.declarations.size());
      for (const auto& entry : tree.declarations) {
        if (const auto* fn = std::get_if<pt::FunctionDecl>(&entry.second)) {
          funcs.push_back(fn);
        }
      }
      std::sort(funcs.begin(), funcs.end(), [](const pt::FunctionDecl* a, const pt::FunctionDecl* b) {
        if (a->location.z != b->location.z) {
          return a->location.z < b->location.z;
        }
        return a->id < b->id;
      });
      return funcs;
    }

    // Spread `count` port dots down an edge at local x == `edgeX`, pushing each
    // anchor (top-to-bottom, in `view`-local coords) into `slot`.
    void emitEdgeDots(double edgeX, const Rect& nodeRect, int count, const Subview& view, std::vector<Vec2>& slot) {
      for (int i = 0; i < count; ++i) {
        const Vec2 anchor{edgeX, nodeRect.y + portFraction(count, i) * nodeRect.h};
        view.renderer().fillRect(view.toScreen(dotRect(anchor)));
        slot.push_back(anchor);
      }
    }

  }  // namespace

  GraphRenderer::GraphRenderer(const Viewport& viewport, Renderer& renderer) :
    viewport_(viewport), renderer_(renderer) { }

  void GraphRenderer::operator()(const pt::ParseTree& tree) {
    for (const pt::FunctionDecl* fn : sortedFunctions(tree)) {
      (*this)(*fn);
    }
  }

  void GraphRenderer::operator()(const pt::FunctionDecl& decl) {
    const FlowGraphLocation& loc = decl.location;
    const Vec2 origin{static_cast<double>(loc.x) * UNIT_PX, static_cast<double>(loc.y) * UNIT_PX};
    const double w = static_cast<double>(loc.width) * UNIT_PX;
    const double h = static_cast<double>(loc.height) * UNIT_PX;

    // Chrome: no clip of its own (an empty function pushes zero clip regions).
    renderer_.drawRect(toScreen(viewport_, atOrigin(origin, Rect{0, 0, w, h})));         // frame
    renderer_.fillRect(toScreen(viewport_, atOrigin(origin, Rect{0, 0, w, kHeaderH})));  // header
    renderer_.drawText(toScreen(viewport_, Vec2{origin.x + kTextPad, origin.y + kTextPad}), decl.name);

    ports_.clear();
    if (decl.input) {
      drawParamRail(origin, *decl.input);
    }
    if (decl.output && decl.output->ret) {
      drawReturnRail(origin, *decl.output->ret, loc.width);
    }

    if (decl.body.nodes.empty() && decl.body.conduits.empty()) {
      return;
    }

    // Body: a Subview clipped to the frame; leaves draw in coords local to it.
    const Subview body{viewport_, origin, Rect{0, 0, w, h}, renderer_};  // pushClip here
    const Subview* const prevBody = body_;
    body_ = &body;

    std::vector<const pt::Node*> nodes;
    nodes.reserve(decl.body.nodes.size());
    for (const auto& entry : decl.body.nodes) {
      nodes.push_back(&entry.second);
    }
    std::sort(nodes.begin(), nodes.end(), [](const pt::Node* a, const pt::Node* b) {
      const auto zOf = [](const pt::Node& n) {
        return std::visit([](const auto& node) { return node.location.z; }, n);
      };
      const auto idOf = [](const pt::Node& n) { return std::visit([](const auto& node) { return node.id; }, n); };
      const int za = zOf(*a);
      const int zb = zOf(*b);
      if (za != zb) {
        return za < zb;
      }
      return idOf(*a) < idOf(*b);
    });
    for (const pt::Node* node : nodes) {
      std::visit(*this, *node);
    }

    std::vector<const pt::Conduit*> conduits;
    conduits.reserve(decl.body.conduits.size());
    for (const auto& entry : decl.body.conduits) {
      conduits.push_back(&entry.second);
    }
    std::sort(
      conduits.begin(), conduits.end(), [](const pt::Conduit* a, const pt::Conduit* b) { return a->id < b->id; });
    for (const pt::Conduit* conduit : conduits) {
      (*this)(*conduit);
    }

    body_ = prevBody;
    // `body` destructs at scope exit -> popClip, after the last conduit line.
  }

  void GraphRenderer::operator()(const pt::Binary& binary) {
    assert(body_);
    const FlowGraphLocation& loc = binary.location;
    const Rect nodeRect{static_cast<double>(loc.x) * UNIT_PX,
                        static_cast<double>(loc.y) * UNIT_PX,
                        static_cast<double>(loc.width) * UNIT_PX,
                        static_cast<double>(loc.height) * UNIT_PX};
    renderer_.drawRect(body_->toScreen(nodeRect));

    PortSet& set = ports_[binary.id];
    const Vec2 textPos{nodeRect.x + kTextPad, nodeRect.y + kTextPad};

    renderer_.drawText(body_->toScreen(textPos), stringify(binary.op));
    emitEdgeDots(nodeRect.x, nodeRect, 2, *body_, set.inputs);
    emitEdgeDots(nodeRect.x + nodeRect.w, nodeRect, 1, *body_, set.outputs);
  }

  void GraphRenderer::operator()(const pt::Unary& unary) {
    assert(body_);
    const FlowGraphLocation& loc = unary.location;
    const Rect nodeRect{static_cast<double>(loc.x) * UNIT_PX,
                        static_cast<double>(loc.y) * UNIT_PX,
                        static_cast<double>(loc.width) * UNIT_PX,
                        static_cast<double>(loc.height) * UNIT_PX};
    renderer_.drawRect(body_->toScreen(nodeRect));

    PortSet& set = ports_[unary.id];
    const Vec2 textPos{nodeRect.x + kTextPad, nodeRect.y + kTextPad};

    renderer_.drawText(body_->toScreen(textPos), stringify(unary.op));
    emitEdgeDots(nodeRect.x, nodeRect, 1, *body_, set.inputs);
    emitEdgeDots(nodeRect.x + nodeRect.w, nodeRect, 1, *body_, set.outputs);
  }

  void GraphRenderer::operator()(const pt::Constant& constant) {
    assert(body_);
    const FlowGraphLocation& loc = constant.location;
    const Rect nodeRect{static_cast<double>(loc.x) * UNIT_PX,
                        static_cast<double>(loc.y) * UNIT_PX,
                        static_cast<double>(loc.width) * UNIT_PX,
                        static_cast<double>(loc.height) * UNIT_PX};
    renderer_.drawRect(body_->toScreen(nodeRect));

    PortSet& set = ports_[constant.id];
    const Vec2 textPos{nodeRect.x + kTextPad, nodeRect.y + kTextPad};

    renderer_.drawText(body_->toScreen(textPos), renderLiteral(constant.value));
    emitEdgeDots(nodeRect.x + nodeRect.w, nodeRect, 1, *body_, set.outputs);
  }

  void GraphRenderer::operator()(const pt::Call& call) {
    assert(body_);
    const FlowGraphLocation& loc = call.location;
    const Rect nodeRect{static_cast<double>(loc.x) * UNIT_PX,
                        static_cast<double>(loc.y) * UNIT_PX,
                        static_cast<double>(loc.width) * UNIT_PX,
                        static_cast<double>(loc.height) * UNIT_PX};
    renderer_.drawRect(body_->toScreen(nodeRect));

    PortSet& set = ports_[call.id];
    const Vec2 textPos{nodeRect.x + kTextPad, nodeRect.y + kTextPad};

    renderer_.drawText(body_->toScreen(textPos), call.target);

    std::vector<const pt::Call::Argument*> args;
    args.reserve(call.arguments.size());
    for (const auto& arg : call.arguments) {
      args.push_back(&arg);
    }
    std::sort(args.begin(), args.end(), [](const pt::Call::Argument* a, const pt::Call::Argument* b) {
      return a->index < b->index;
    });

    // One row per argument, stacked below the header row (kRailStep tall),
    // matching CallNode.tsx's column of CallArgumentNode rows; each row's
    // input handle sits at the row's vertical centre.
    for (std::size_t k = 0; k < args.size(); ++k) {
      const double rowTop = nodeRect.y + (static_cast<double>(k) + 1.0) * kRailStep;
      renderer_.drawText(body_->toScreen(Vec2{nodeRect.x + kTextPad, rowTop + kTextPad}), args[k]->name);
      const Vec2 anchor{nodeRect.x, rowTop + kRailStep * 0.5};
      renderer_.fillRect(body_->toScreen(dotRect(anchor)));
      set.inputs.push_back(anchor);
    }

    if (call._return.has_value()) {
      emitEdgeDots(nodeRect.x + nodeRect.w, nodeRect, 1, *body_, set.outputs);
    }
  }

  void GraphRenderer::operator()(const pt::Conduit& conduit) {
    assert(body_);
    const auto sourceIt = ports_.find(conduit.input);
    if (sourceIt == ports_.end() || sourceIt->second.outputs.empty()) {
      return;
    }
    // createEdges (createNodes.ts): sourceHandle is always `input-<conduit.input>-0`,
    // i.e. the source node's output port 0; targetHandle is
    // `output-<child.target>-<child.index>`, i.e. the target's input port child.index.
    const Vec2 source = sourceIt->second.outputs.front();
    for (const pt::Conduit::Output& child : conduit.children) {
      const auto targetIt = ports_.find(child.target);
      if (targetIt == ports_.end()) {
        continue;
      }
      if (child.index < 0 || static_cast<std::size_t>(child.index) >= targetIt->second.inputs.size()) {
        continue;
      }
      const Vec2 target = targetIt->second.inputs[static_cast<std::size_t>(child.index)];
      renderer_.drawLine(body_->toScreen(source), body_->toScreen(target));
    }
  }

  void GraphRenderer::drawParamRail(Vec2 origin, const pt::FunctionDecl::InputBlock& input) {
    std::vector<const pt::FunctionDecl::Parameter*> params;
    params.reserve(input.parameters.size());
    for (const auto& param : input.parameters) {
      params.push_back(&param);
    }
    std::sort(
      params.begin(), params.end(), [](const pt::FunctionDecl::Parameter* a, const pt::FunctionDecl::Parameter* b) {
        return a->index < b->index;
      });

    for (std::size_t i = 0; i < params.size(); ++i) {
      const pt::FunctionDecl::Parameter& param = *params[i];
      const Rect rect{0.0, (static_cast<double>(i) + 1.0) * kRailStep, kParamW, kRailStep};  // local
      renderer_.drawRect(toScreen(viewport_, atOrigin(origin, rect)));
      renderer_.drawText(toScreen(viewport_, Vec2{origin.x + rect.x + kTextPad, origin.y + rect.y + kTextPad}),
                         param.typeName + " " + param.name);
      const Vec2 anchor{rect.x + rect.w, rect.y + rect.h * 0.5};  // local
      renderer_.fillRect(toScreen(viewport_, atOrigin(origin, dotRect(anchor))));
      ports_[param.id].outputs.push_back(anchor);
    }
  }

  void GraphRenderer::drawReturnRail(Vec2 origin, const pt::FunctionDecl::Return& ret, int width) {
    const Rect rect{
      (static_cast<double>(width) - kReturnInsetLogical) * UNIT_PX, kRailStep, kRailStep, kRailStep};  // local
    renderer_.drawRect(toScreen(viewport_, atOrigin(origin, rect)));
    renderer_.drawText(toScreen(viewport_, Vec2{origin.x + rect.x + kTextPad, origin.y + rect.y + kTextPad}),
                       ret.typeName);
    const Vec2 anchor{rect.x, rect.y + rect.h * 0.5};  // local
    renderer_.fillRect(toScreen(viewport_, atOrigin(origin, dotRect(anchor))));
    ports_[ret.id].inputs.push_back(anchor);
  }

  void renderGraph(const pt::ParseTree& tree, const Viewport& view, Renderer& renderer) {
    GraphRenderer{view, renderer}(tree);
  }

  Rect graphBounds(const pt::ParseTree& tree) {
    bool any = false;
    double minX = 0.0;
    double minY = 0.0;
    double maxX = 0.0;
    double maxY = 0.0;
    for (const auto& entry : tree.declarations) {
      const auto* fn = std::get_if<pt::FunctionDecl>(&entry.second);
      if (fn == nullptr) {
        continue;
      }
      const FlowGraphLocation& loc = fn->location;
      const double x0 = static_cast<double>(loc.x) * UNIT_PX;
      const double y0 = static_cast<double>(loc.y) * UNIT_PX;
      const double x1 = x0 + static_cast<double>(loc.width) * UNIT_PX;
      const double y1 = y0 + static_cast<double>(loc.height) * UNIT_PX;
      if (!any) {
        minX = x0;
        minY = y0;
        maxX = x1;
        maxY = y1;
        any = true;
      } else {
        minX = std::min(minX, x0);
        minY = std::min(minY, y0);
        maxX = std::max(maxX, x1);
        maxY = std::max(maxY, y1);
      }
    }
    if (!any) {
      return {0.0, 0.0, 0.0, 0.0};
    }
    return {minX, minY, maxX - minX, maxY - minY};
  }

}  // namespace fluir::editor
