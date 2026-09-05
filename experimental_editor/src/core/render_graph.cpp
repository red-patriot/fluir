#include "editor/core/render_graph.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <variant>
#include <vector>

#include "compiler/models/id.hpp"
#include "editor/actors/node_actor.hpp"
#include "editor/core/graph_geometry.hpp"

namespace fluir::editor {

  namespace {

    // Chrome (frame / header / name / rails) draws with no clip of its own, so it
    // uses the bare viewport transform rather than a Subview (which always
    // clips). `world == origin + local`.
    Vec2 toScreen(const Viewport& vp, Vec2 world) { return vp.worldToScreen(world); }

    Rect toScreen(const Viewport& vp, Rect world) {
      const Vec2 tl = vp.worldToScreen(world.topLeft());
      return {tl.x, tl.y, world.w * vp.scale, world.h * vp.scale};
    }

    fluir::ID idOf(const pt::Node& node) {
      return std::visit([](const auto& n) { return n.id; }, node);
    }

  }  // namespace

  GraphRenderer::GraphRenderer(const EditorContext& ctx,
                               const Viewport& viewport,
                               Renderer& renderer,
                               const GraphScene& scene) :
    ctx_(ctx), viewport_(viewport), renderer_(renderer), scene_(scene) { }

  void GraphRenderer::operator()(const pt::ParseTree& tree) {
    for (const pt::FunctionDecl* fn : sortedFunctions(tree)) {
      (*this)(*fn);
    }
  }

  void GraphRenderer::operator()(const pt::FunctionDecl& decl) {
    const FlowGraphLocation& loc = decl.location;
    const Vec2 origin = functionOrigin(loc, ctx_.layout.unitPx);
    const double w = static_cast<double>(loc.width) * ctx_.layout.unitPx;
    const double h = static_cast<double>(loc.height) * ctx_.layout.unitPx;

    ports_.clear();

    const Subview frame{viewport_, Rect{origin.x, origin.y, w, h}, renderer_};
    Actor* frameActor = scene_.find(decl.id);
    assert(frameActor);
    frameActor->draw(frame, ctx_);

    // Body content is offset below the header band so nodes don't render over it.
    const Vec2 bodyOrigin_ = bodyOrigin(origin, ctx_.layout.headerH());
    const Subview body = frame.child(Rect{0.0, ctx_.layout.headerH(), w, h});
    const Subview* const prevBody = body_;
    body_ = &body;

    if (decl.input) {
      drawParamRail(bodyOrigin_, *decl.input);
    }
    if (decl.output && decl.output->ret) {
      drawReturnRail(bodyOrigin_, *decl.output->ret, loc.width);
    }

    if (decl.body.nodes.empty() && decl.body.conduits.empty()) {
      body_ = prevBody;  // TODO: use scope guard here
      return;
    }

    for (const pt::Node* node : sortedNodes(decl.body)) {
      const fluir::ID id = idOf(*node);
      Actor* actor = scene_.find(decl.id, id);
      assert(actor);
      actor->draw(*body_, ctx_);
      // scene_.find(functionId, nodeId) only ever resolves byId_ entries, which are always NodeActor.
      ports_[id] = static_cast<NodeActor*>(actor)->ports(ctx_);
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
      renderer_.drawLine(body_->toScreen(source), body_->toScreen(target), ctx_.theme.conduit);
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
      const Rect rect{
        0.0, static_cast<double>(i) * ctx_.layout.railStep(), ctx_.layout.paramW(), ctx_.layout.railStep()};  // local
      renderer_.fillRect(toScreen(viewport_, atOrigin(origin, rect)), ctx_.theme.funcDeclHeader);
      renderer_.drawRect(toScreen(viewport_, atOrigin(origin, rect)), ctx_.theme.border);
      renderer_.drawText(
        toScreen(viewport_, Vec2{origin.x + rect.x + ctx_.layout.textPad, origin.y + rect.y + ctx_.layout.textPad}),
        param.typeName + " " + param.name,
        ctx_.theme.text);
      const Vec2 anchor{rect.x + rect.w, rect.y + rect.h * 0.5};  // local
      renderer_.fillRect(toScreen(viewport_, atOrigin(origin, dotRect(anchor, ctx_.layout.portDot))),
                         ctx_.theme.border);
      ports_[param.id].outputs.push_back(anchor);
    }
  }

  void GraphRenderer::drawReturnRail(Vec2 origin, const pt::FunctionDecl::Return& ret, int width) {
    const Rect rect{(static_cast<double>(width) - ctx_.layout.returnInsetUnits) * ctx_.layout.unitPx,
                    0,
                    ctx_.layout.railStep(),
                    ctx_.layout.railStep()};
    renderer_.fillRect(toScreen(viewport_, atOrigin(origin, rect)), ctx_.theme.funcDeclHeader);
    renderer_.drawRect(toScreen(viewport_, atOrigin(origin, rect)), ctx_.theme.border);
    renderer_.drawText(
      toScreen(viewport_, Vec2{origin.x + rect.x + ctx_.layout.textPad, origin.y + rect.y + ctx_.layout.textPad}),
      ret.typeName,
      ctx_.theme.text);
    const Vec2 anchor{rect.x, rect.y + rect.h * 0.5};  // local
    renderer_.fillRect(toScreen(viewport_, atOrigin(origin, dotRect(anchor, ctx_.layout.portDot))), ctx_.theme.border);
    ports_[ret.id].inputs.push_back(anchor);
  }

  void renderGraph(const EditorContext& ctx, const pt::ParseTree& tree, const Viewport& view, Renderer& renderer) {
    GraphScene scene;
    scene.build(ctx, tree);
    GraphRenderer{ctx, view, renderer, scene}(tree);
  }

  Rect graphBounds(const EditorContext& ctx, const pt::ParseTree& tree) {
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
      const double x0 = static_cast<double>(loc.x) * ctx.layout.unitPx;
      const double y0 = static_cast<double>(loc.y) * ctx.layout.unitPx;
      const double x1 = x0 + static_cast<double>(loc.width) * ctx.layout.unitPx;
      const double y1 = y0 + static_cast<double>(loc.height) * ctx.layout.unitPx;
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
