#include "editor/actors/rail_actors.hpp"

#include <algorithm>
#include <cstddef>
#include <utility>

#include "editor/actors/function_decl_actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/graph_geometry.hpp"
#include "editor/core/renderer.hpp"
#include "editor/core/viewport.hpp"

namespace fluir::editor {

  namespace {

    // Rail chrome: fill, border, label at textPad, then the port dot.
    void drawRail(
      const Subview& body, const EditorContext& ctx, const Rect& rect, const std::string& label, Vec2 anchor) {
      body.renderer().fillRect(body.toScreen(rect), ctx.theme.funcDeclHeader);
      body.renderer().drawRect(body.toScreen(rect), ctx.theme.border);
      body.renderer().drawText(
        body.toScreen(Vec2{rect.x + ctx.layout.textPad, rect.y + ctx.layout.textPad}), label, ctx.theme.text);
      body.renderer().fillRect(body.toScreen(dotRect(anchor, ctx.layout.portDot)), ctx.theme.border);
    }

    // The nearest FunctionDeclActor ancestor owns the port registry a conduit resolves through.
    const FunctionDeclActor* enclosingFrame(const Actor* actor) {
      for (const Actor* a = actor; a != nullptr; a = a->parent()) {
        if (const auto* frame = dynamic_cast<const FunctionDeclActor*>(a)) {
          return frame;
        }
      }
      return nullptr;
    }

  }  // namespace

  ParameterActor::ParameterActor(const pt::FunctionDecl::Parameter& param, std::size_t row) :
    PortActor(param.id, Rect{}), param_(param), row_(row) { }

  void ParameterActor::layout(const EditorContext& ctx) {
    setBounds(Rect{0.0, static_cast<double>(row_) * ctx.layout.railStep(), ctx.layout.paramW(), ctx.layout.railStep()});
    Actor::layout(ctx);
  }

  PortSet ParameterActor::ports(const EditorContext& ctx) const {
    const Rect& rect = bounds();
    return {{}, {Vec2{rect.x + rect.w, rect.y + rect.h * 0.5}}};
  }

  void ParameterActor::drawSelf(const Subview& body, const EditorContext& ctx) const {
    drawRail(body, ctx, bounds(), param_.typeName + " " + param_.name, ports(ctx).outputs.front());
  }

  ReturnActor::ReturnActor(const pt::FunctionDecl::Return& ret) : PortActor(ret.id, Rect{}), ret_(ret) { }

  void ReturnActor::layout(const EditorContext& ctx) {
    setBounds(Rect{(static_cast<double>(frameWidthUnits_) - ctx.layout.returnInsetUnits) * ctx.layout.unitPx,
                   0.0,
                   ctx.layout.railStep(),
                   ctx.layout.railStep()});
    Actor::layout(ctx);
  }

  PortSet ReturnActor::ports(const EditorContext& ctx) const {
    const Rect& rect = bounds();
    return {{Vec2{rect.x, rect.y + rect.h * 0.5}}, {}};
  }

  void ReturnActor::drawSelf(const Subview& body, const EditorContext& ctx) const {
    drawRail(body, ctx, bounds(), ret_.typeName, ports(ctx).inputs.front());
  }

  ConduitActor::ConduitActor(fluir::ID functionId, const pt::Conduit& conduit) :
    Actor(Rect{}), functionId_(functionId), id_(conduit.id), index_(conduit.index), sourceId_(conduit.input) {
    targets_.reserve(conduit.children.size());
    for (const pt::Conduit::Output& child : conduit.children) {
      targets_.push_back({child.target, child.index});
    }
  }

  pt::Conduit ConduitActor::conduit() const {
    pt::Conduit conduit{.id = id_, .input = sourceId_, .index = index_, .children = {}};
    conduit.children.reserve(targets_.size());
    for (const Endpoint& target : targets_) {
      conduit.children.push_back({.target = target.target, .index = target.index});
    }
    return conduit;
  }

  void ConduitActor::layout(const EditorContext& ctx) {
    lines_.clear();
    // Endpoints are ids, resolved late: an id naming nothing is a gap in the drawing.
    const FunctionDeclActor* frame = enclosingFrame(this);
    const PortActor* sourceActor = frame == nullptr ? nullptr : frame->port(sourceId_);
    if (sourceActor == nullptr) {
      setBounds(Rect{});
      return;
    }
    const PortSet sourcePorts = sourceActor->ports(ctx);
    if (sourcePorts.outputs.empty()) {
      setBounds(Rect{});
      return;
    }
    const Vec2 source = sourcePorts.outputs.front();
    for (const Endpoint& target : targets_) {
      const PortActor* targetActor = frame->port(target.target);
      if (targetActor == nullptr) {
        continue;
      }
      const PortSet targetPorts = targetActor->ports(ctx);
      if (target.index < 0 || static_cast<std::size_t>(target.index) >= targetPorts.inputs.size()) {
        continue;
      }
      lines_.emplace_back(source, targetPorts.inputs[static_cast<std::size_t>(target.index)]);
    }
    if (lines_.empty()) {
      setBounds(Rect{});
      return;
    }
    double minX = source.x;
    double minY = source.y;
    double maxX = source.x;
    double maxY = source.y;
    for (const auto& [from, to] : lines_) {
      for (const Vec2& p : {from, to}) {
        minX = std::min(minX, p.x);
        minY = std::min(minY, p.y);
        maxX = std::max(maxX, p.x);
        maxY = std::max(maxY, p.y);
      }
    }
    setBounds(Rect{minX, minY, maxX - minX, maxY - minY});
  }

  void ConduitActor::drawSelf(const Subview& body, const EditorContext& ctx) const {
    for (const auto& [from, to] : lines_) {
      body.renderer().drawLine(body.toScreen(from), body.toScreen(to), ctx.theme.conduit);
    }
  }

}  // namespace fluir::editor
