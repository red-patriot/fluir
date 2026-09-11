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

      if (fn->input) {
        std::vector<const pt::FunctionDecl::Parameter*> params;
        params.reserve(fn->input->parameters.size());
        for (const auto& param : fn->input->parameters) {
          params.push_back(&param);
        }
        std::sort(params.begin(), params.end(), [](const auto* a, const auto* b) { return a->index < b->index; });
        for (std::size_t row = 0; row < params.size(); ++row) {
          auto& rail = frameActor.body().add(std::make_unique<ParameterActor>(*params[row], row));
          frameActor.registerPort(rail);
        }
      }

      if (fn->output && fn->output->ret) {
        auto& rail = frameActor.body().add(std::make_unique<ReturnActor>(*fn->output->ret));
        frameActor.setReturnActor(rail);
        frameActor.registerPort(rail);
      }

      for (const pt::Node* node : sortedNodes(fn->body)) {
        const Rect bounds = localRect(locationOf(*node), ctx.layout.unitPx);
        auto& nodeActor = frameActor.body().add(std::visit(MakeActor{fn->id, bounds}, *node));
        byId_[nodeActor.id()] = &nodeActor;
        frameActor.registerPort(nodeActor);
      }

      // Endpoints stay as ids: a dangling one is a legitimate authoring state, not a drop.
      for (const pt::Conduit* conduit : sortedConduits(fn->body)) {
        auto& conduitActor = frameActor.body().add(std::make_unique<ConduitActor>(fn->id, *conduit));
        byId_[*conduitActor.selectionId()] = &conduitActor;
      }
    }
    layout(ctx);
    if (wasSelected) {
      select(*wasSelected);
    }
  }

  GraphScene::DetachedActor GraphScene::detach(const fluir::FullID& id) {
    Actor* actor = resolve(id);
    if (actor == nullptr) {
      return {};
    }
    // A stale selection would outlive the actor it names, so drop it here.
    if (selected_ && !selected_->empty() && !id.empty() && selected_->front() == id.front()) {
      clearSelection();
    }

    if (id.size() == 1) {
      auto* frame = static_cast<FunctionDeclActor*>(actor);
      DetachedActor detached{nullptr, {}, root_->indexOf(*frame)};
      // The body goes with the frame, so every id it owns leaves the index too.
      std::erase_if(byId_, [&id](const auto& entry) { return entry.first.front() == id.front(); });
      frames_.erase(frame->functionId());
      detached.actor = root_->detach(*frame);
      return detached;
    }

    auto* frame = dynamic_cast<FunctionDeclActor*>(find(id[0]));
    if (frame == nullptr) {
      return {};
    }
    DetachedActor detached{nullptr, fluir::FullID{id[0]}, frame->body().indexOf(*actor)};
    byId_.erase(id);
    if (auto* port = dynamic_cast<PortActor*>(actor)) {
      frame->unregisterPort(port->portId());
    }
    detached.actor = frame->body().detach(*actor);
    return detached;
  }

  bool GraphScene::attach(DetachedActor detached) {
    if (detached.actor == nullptr) {
      return false;
    }
    if (detached.parent.empty()) {
      auto* frame = dynamic_cast<FunctionDeclActor*>(detached.actor.get());
      if (frame == nullptr) {
        return false;
      }
      frames_[frame->functionId()] = frame;
      for (const auto& child : frame->body().children()) {
        if (const std::optional<fluir::FullID> childId = child->selectionId()) {
          byId_[*childId] = child.get();
        }
      }
      root_->insert(detached.index, std::move(detached.actor));
      return true;
    }

    auto* frame = dynamic_cast<FunctionDeclActor*>(find(detached.parent.front()));
    if (frame == nullptr) {
      return false;
    }
    Actor* actor = detached.actor.get();
    if (const std::optional<fluir::FullID> id = actor->selectionId()) {
      byId_[*id] = actor;
    }
    if (auto* port = dynamic_cast<PortActor*>(actor)) {
      frame->registerPort(*port);
    }
    frame->body().insert(detached.index, std::move(detached.actor));
    return true;
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
