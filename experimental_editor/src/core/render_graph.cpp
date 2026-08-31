#include "editor/core/render_graph.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <unordered_map>
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

    // World-space port anchors for one graph element (param / return / body
    // node), keyed by element id. Wires read these back to find their endpoints.
    struct PortSet {
      std::vector<Vec2> inputs;
      std::vector<Vec2> outputs;
    };
    using PortMap = std::unordered_map<fluir::ID, PortSet>;

    Rect dotRect(Vec2 anchor) { return {anchor.x - kPortDot * 0.5, anchor.y - kPortDot * 0.5, kPortDot, kPortDot}; }

    Vec2 toScreen(const Viewport& vp, Vec2 world) { return vp.worldToScreen(world); }

    Rect toScreen(const Viewport& vp, Rect world) {
      const Vec2 tl = vp.worldToScreen(world.topLeft());
      return {tl.x, tl.y, world.w * vp.scale, world.h * vp.scale};
    }

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

    const FlowGraphLocation& nodeLocation(const pt::Node& node) {
      return std::visit([](const auto& n) -> const FlowGraphLocation& { return n.location; }, node);
    }

    fluir::ID nodeId(const pt::Node& node) {
      return std::visit([](const auto& n) { return n.id; }, node);
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

    void emitParamRail(
      const pt::FunctionDecl::InputBlock& input, Vec2 origin, const Viewport& vp, Renderer& out, PortMap& ports) {
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
        const Rect rect{origin.x, origin.y + (static_cast<double>(i) + 1.0) * kRailStep, kParamW, kRailStep};
        out.drawRect(toScreen(vp, rect));
        out.drawText(toScreen(vp, Vec2{rect.x + kTextPad, rect.y + kTextPad}), param.typeName + " " + param.name);
        const Vec2 anchor{rect.x + rect.w, rect.y + rect.h * 0.5};
        out.fillRect(toScreen(vp, dotRect(anchor)));
        ports[param.id].outputs.push_back(anchor);
      }
    }

    void emitReturnRail(
      const pt::FunctionDecl::Return& ret, Vec2 origin, int width, const Viewport& vp, Renderer& out, PortMap& ports) {
      const Rect rect{origin.x + (static_cast<double>(width) - kReturnInsetLogical) * UNIT_PX,
                      origin.y + kRailStep,
                      kRailStep,
                      kRailStep};
      out.drawRect(toScreen(vp, rect));
      out.drawText(toScreen(vp, Vec2{rect.x + kTextPad, rect.y + kTextPad}), ret.typeName);
      const Vec2 anchor{rect.x, rect.y + rect.h * 0.5};
      out.fillRect(toScreen(vp, dotRect(anchor)));
      ports[ret.id].inputs.push_back(anchor);
    }

    // Spread `count` port dots down an edge at world x == `edgeX`, pushing each
    // anchor (top-to-bottom) into `slot`.
    void emitEdgeDots(
      double edgeX, const Rect& nodeRect, int count, const Viewport& vp, Renderer& out, std::vector<Vec2>& slot) {
      for (int i = 0; i < count; ++i) {
        const Vec2 anchor{edgeX, nodeRect.y + portFraction(count, i) * nodeRect.h};
        out.fillRect(toScreen(vp, dotRect(anchor)));
        slot.push_back(anchor);
      }
    }

    void emitBodyNode(const pt::Node& node, Vec2 origin, const Viewport& vp, Renderer& out, PortMap& ports) {
      const FlowGraphLocation& loc = nodeLocation(node);
      const Rect nodeRect{origin.x + static_cast<double>(loc.x) * UNIT_PX,
                          origin.y + static_cast<double>(loc.y) * UNIT_PX,
                          static_cast<double>(loc.width) * UNIT_PX,
                          static_cast<double>(loc.height) * UNIT_PX};
      out.drawRect(toScreen(vp, nodeRect));

      PortSet& set = ports[nodeId(node)];
      const Vec2 textPos{nodeRect.x + kTextPad, nodeRect.y + kTextPad};

      if (const auto* constant = std::get_if<pt::Constant>(&node)) {
        out.drawText(toScreen(vp, textPos), renderLiteral(constant->value));
        emitEdgeDots(nodeRect.x + nodeRect.w, nodeRect, 1, vp, out, set.outputs);
        return;
      }
      if (const auto* unary = std::get_if<pt::Unary>(&node)) {
        out.drawText(toScreen(vp, textPos), stringify(unary->op));
        emitEdgeDots(nodeRect.x, nodeRect, 1, vp, out, set.inputs);
        emitEdgeDots(nodeRect.x + nodeRect.w, nodeRect, 1, vp, out, set.outputs);
        return;
      }
      if (const auto* binary = std::get_if<pt::Binary>(&node)) {
        out.drawText(toScreen(vp, textPos), stringify(binary->op));
        emitEdgeDots(nodeRect.x, nodeRect, 2, vp, out, set.inputs);
        emitEdgeDots(nodeRect.x + nodeRect.w, nodeRect, 1, vp, out, set.outputs);
        return;
      }
      if (const auto* call = std::get_if<pt::Call>(&node)) {
        out.drawText(toScreen(vp, textPos), call->target);

        std::vector<const pt::Call::Argument*> args;
        args.reserve(call->arguments.size());
        for (const auto& arg : call->arguments) {
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
          out.drawText(toScreen(vp, Vec2{nodeRect.x + kTextPad, rowTop + kTextPad}), args[k]->name);
          const Vec2 anchor{nodeRect.x, rowTop + kRailStep * 0.5};
          out.fillRect(toScreen(vp, dotRect(anchor)));
          set.inputs.push_back(anchor);
        }

        if (call->_return.has_value()) {
          emitEdgeDots(nodeRect.x + nodeRect.w, nodeRect, 1, vp, out, set.outputs);
        }
        return;
      }
    }

    void emitWires(const pt::Block& body, const Viewport& vp, Renderer& out, const PortMap& ports) {
      std::vector<const pt::Conduit*> conduits;
      conduits.reserve(body.conduits.size());
      for (const auto& entry : body.conduits) {
        conduits.push_back(&entry.second);
      }
      std::sort(
        conduits.begin(), conduits.end(), [](const pt::Conduit* a, const pt::Conduit* b) { return a->id < b->id; });

      for (const pt::Conduit* conduit : conduits) {
        const auto sourceIt = ports.find(conduit->input);
        if (sourceIt == ports.end() || sourceIt->second.outputs.empty()) {
          continue;
        }
        // createEdges (createNodes.ts): sourceHandle is always `input-<conduit.input>-0`,
        // i.e. the source node's output port 0; targetHandle is
        // `output-<child.target>-<child.index>`, i.e. the target's input port child.index.
        const Vec2 source = sourceIt->second.outputs.front();
        for (const pt::Conduit::Output& child : conduit->children) {
          const auto targetIt = ports.find(child.target);
          if (targetIt == ports.end()) {
            continue;
          }
          if (child.index < 0 || static_cast<std::size_t>(child.index) >= targetIt->second.inputs.size()) {
            continue;
          }
          const Vec2 target = targetIt->second.inputs[static_cast<std::size_t>(child.index)];
          out.drawLine(toScreen(vp, source), toScreen(vp, target));
        }
      }
    }

  }  // namespace

  void renderGraph(const pt::ParseTree& tree, const Viewport& view, Renderer& renderer) {
    for (const pt::FunctionDecl* fn : sortedFunctions(tree)) {
      const FlowGraphLocation& loc = fn->location;
      const Vec2 origin{static_cast<double>(loc.x) * UNIT_PX, static_cast<double>(loc.y) * UNIT_PX};

      const Rect frame{
        origin.x, origin.y, static_cast<double>(loc.width) * UNIT_PX, static_cast<double>(loc.height) * UNIT_PX};
      renderer.drawRect(toScreen(view, frame));

      const Rect header{origin.x, origin.y, static_cast<double>(loc.width) * UNIT_PX, kHeaderH};
      renderer.fillRect(toScreen(view, header));
      renderer.drawText(toScreen(view, Vec2{origin.x + kTextPad, origin.y + kTextPad}), fn->name);

      PortMap ports;
      if (fn->input) {
        emitParamRail(*fn->input, origin, view, renderer, ports);
      }
      if (fn->output && fn->output->ret) {
        emitReturnRail(*fn->output->ret, origin, loc.width, view, renderer, ports);
      }

      if (fn->body.nodes.empty() && fn->body.conduits.empty()) {
        continue;
      }

      renderer.pushClip(toScreen(view, frame));

      std::vector<const pt::Node*> nodes;
      nodes.reserve(fn->body.nodes.size());
      for (const auto& entry : fn->body.nodes) {
        nodes.push_back(&entry.second);
      }
      std::sort(nodes.begin(), nodes.end(), [](const pt::Node* a, const pt::Node* b) {
        const int za = nodeLocation(*a).z;
        const int zb = nodeLocation(*b).z;
        if (za != zb) {
          return za < zb;
        }
        return nodeId(*a) < nodeId(*b);
      });
      for (const pt::Node* node : nodes) {
        emitBodyNode(*node, origin, view, renderer, ports);
      }

      emitWires(fn->body, view, renderer, ports);

      renderer.popClip();
    }
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
