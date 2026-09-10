#include "editor/actors/scene.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "compiler/models/location.hpp"
#include "editor/actors/function_decl_actor.hpp"
#include "editor/actors/node_actor.hpp"
#include "editor/actors/node_actors.hpp"
#include "editor/actors/port_actor.hpp"
#include "editor/actors/rail_actors.hpp"
#include "editor/core/graph_geometry.hpp"

namespace fluir::editor {
  namespace {

    // Construction-time switch from a pt::Node alternative to its matching concrete Actor
    // subclass. This is a one-shot factory decision, not the actors' own dispatch (which
    // stays virtual via Actor::onClick).
    struct MakeActor {
      fluir::ID functionId;
      Rect bounds;

      std::unique_ptr<NodeActor> operator()(const pt::Binary& node) const {
        return std::make_unique<BinaryActor>(functionId, node, bounds);
      }
      std::unique_ptr<NodeActor> operator()(const pt::Unary& node) const {
        return std::make_unique<UnaryActor>(functionId, node, bounds);
      }
      std::unique_ptr<NodeActor> operator()(const pt::Constant& node) const {
        return std::make_unique<ConstantActor>(functionId, node, bounds);
      }
      std::unique_ptr<NodeActor> operator()(const pt::Call& node) const {
        return std::make_unique<CallActor>(functionId, node, bounds);
      }
    };

    const FlowGraphLocation& locationOf(const pt::Node& node) {
      return std::visit([](const auto& n) -> const FlowGraphLocation& { return n.location; }, node);
    }

  }  // namespace

  void GraphScene::clear() {
    root_->clearChildren();
    byId_.clear();
    frames_.clear();
    selected_.reset();
  }

  void GraphScene::build(const EditorContext& ctx, const pt::ParseTree& tree) {
    // Selection is an id, so it outlives the actors clear() destroys.
    const std::optional<fluir::FullID> wasSelected = selected_;
    clear();
    for (const pt::FunctionDecl* fn : sortedFunctions(tree)) {
      const Rect frame = localRect(fn->location, ctx.layout.unitPx);
      auto& frameActor = root_->add(std::make_unique<FunctionDeclActor>(*fn, frame));
      frames_[frameActor.functionId()] = &frameActor;

      // Every conduit endpoint in this function, by body-scope id: rails and nodes alike.
      std::unordered_map<fluir::ID, PortActor*> ports;

      if (fn->input) {
        std::vector<const pt::FunctionDecl::Parameter*> params;
        params.reserve(fn->input->parameters.size());
        for (const auto& param : fn->input->parameters) {
          params.push_back(&param);
        }
        std::sort(params.begin(), params.end(), [](const auto* a, const auto* b) { return a->index < b->index; });
        for (std::size_t row = 0; row < params.size(); ++row) {
          auto& rail = frameActor.body().add(std::make_unique<ParameterActor>(*params[row], row));
          ports[rail.portId()] = &rail;
        }
      }

      if (fn->output && fn->output->ret) {
        auto& rail = frameActor.body().add(std::make_unique<ReturnActor>(*fn->output->ret));
        frameActor.setReturnActor(rail);
        ports[rail.portId()] = &rail;
      }

      for (const pt::Node* node : sortedNodes(fn->body)) {
        const Rect bounds = localRect(locationOf(*node), ctx.layout.unitPx);
        auto& nodeActor = frameActor.body().add(std::visit(MakeActor{fn->id, bounds}, *node));
        byId_[nodeActor.id()] = &nodeActor;
        ports[nodeActor.portId()] = &nodeActor;
      }

      for (const pt::Conduit* conduit : sortedConduits(fn->body)) {
        const auto source = ports.find(conduit->input);
        if (source == ports.end()) {
          continue;  // a dangling source draws nothing; decide it once, not every frame
        }
        std::vector<ConduitActor::Target> targets;
        for (const pt::Conduit::Output& child : conduit->children) {
          const auto target = ports.find(child.target);
          if (target != ports.end()) {
            targets.push_back({target->second, child.index});
          }
        }
        frameActor.body().add(std::make_unique<ConduitActor>(*source->second, std::move(targets)));
      }
    }
    layout(ctx);
    if (wasSelected) {
      select(*wasSelected);
    }
  }

  Rect GraphScene::worldBounds() const {
    const auto& frames = root_->children();
    if (frames.empty()) {
      return {0.0, 0.0, 0.0, 0.0};
    }
    Rect box = frames.front()->worldBounds();
    double minX = box.x;
    double minY = box.y;
    double maxX = box.x + box.w;
    double maxY = box.y + box.h;
    for (const auto& frame : frames) {
      const Rect r = frame->worldBounds();
      minX = std::min(minX, r.x);
      minY = std::min(minY, r.y);
      maxX = std::max(maxX, r.x + r.w);
      maxY = std::max(maxY, r.y + r.h);
    }
    return {minX, minY, maxX - minX, maxY - minY};
  }

  Actor* GraphScene::topmostAt(Vec2 worldPos) const { return root_->hitTest(worldPos); }

  Actor* GraphScene::find(fluir::ID functionId, fluir::ID nodeId) const {
    const auto it = byId_.find(fluir::FullID{functionId, nodeId});
    return it == byId_.end() ? nullptr : it->second;
  }

  Actor* GraphScene::find(fluir::ID functionId) const {
    const auto it = frames_.find(functionId);
    return it == frames_.end() ? nullptr : it->second;
  }

  Actor* GraphScene::resolve(const fluir::FullID& id) const {
    if (id.size() == 1) {
      return find(id[0]);
    }
    return id.size() == 2 ? find(id[0], id[1]) : nullptr;
  }

  void GraphScene::select(fluir::FullID id) {
    Actor* actor = resolve(id);
    if (actor == nullptr) {
      return;
    }
    clearSelection();
    actor->setSelected(true);
    selected_ = std::move(id);
  }

  void GraphScene::clearSelection() {
    if (!selected_) {
      return;
    }
    if (Actor* actor = resolve(*selected_)) {
      actor->setSelected(false);
    }
    selected_.reset();
  }

}  // namespace fluir::editor
