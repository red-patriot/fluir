#include "editor/actors/scene.hpp"

#include <algorithm>
#include <memory>
#include <variant>

#include "compiler/models/location.hpp"
#include "editor/actors/function_decl_actor.hpp"
#include "editor/actors/node_actor.hpp"
#include "editor/actors/node_actors.hpp"
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
  }

  void GraphScene::build(const EditorContext& ctx, const pt::ParseTree& tree) {
    clear();
    for (const pt::FunctionDecl* fn : sortedFunctions(tree)) {
      const Rect frame = localRect(fn->location, ctx.layout.unitPx);
      auto& frameActor = root_->add(std::make_unique<FunctionDeclActor>(*fn, frame));
      frames_[frameActor.functionId()] = &frameActor;

      for (const pt::Node* node : sortedNodes(fn->body)) {
        const Rect bounds = localRect(locationOf(*node), ctx.layout.unitPx);
        auto& nodeActor = frameActor.body().add(std::visit(MakeActor{fn->id, bounds}, *node));
        byId_[nodeActor.id()] = &nodeActor;
      }
    }
    layout(ctx);
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

}  // namespace fluir::editor
