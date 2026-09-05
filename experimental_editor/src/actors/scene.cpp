#include "editor/actors/scene.hpp"

#include <memory>
#include <variant>

#include "compiler/models/location.hpp"
#include "editor/actors/function_decl_actor.hpp"
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

      std::unique_ptr<Actor> operator()(const pt::Binary& node) const {
        return std::make_unique<BinaryActor>(functionId, node, bounds);
      }
      std::unique_ptr<Actor> operator()(const pt::Unary& node) const {
        return std::make_unique<UnaryActor>(functionId, node, bounds);
      }
      std::unique_ptr<Actor> operator()(const pt::Constant& node) const {
        return std::make_unique<ConstantActor>(functionId, node, bounds);
      }
      std::unique_ptr<Actor> operator()(const pt::Call& node) const {
        return std::make_unique<CallActor>(functionId, node, bounds);
      }
    };

    const FlowGraphLocation& locationOf(const pt::Node& node) {
      return std::visit([](const auto& n) -> const FlowGraphLocation& { return n.location; }, node);
    }

  }  // namespace

  void GraphScene::clear() {
    actors_.clear();
    byId_.clear();
  }

  void GraphScene::build(const EditorContext& ctx, const pt::ParseTree& tree) {
    clear();
    for (const pt::FunctionDecl* fn : sortedFunctions(tree)) {
      const Vec2 frameOrigin = functionOrigin(fn->location, ctx.layout.unitPx);
      const double w = static_cast<double>(fn->location.width) * ctx.layout.unitPx;
      const double h = static_cast<double>(fn->location.height) * ctx.layout.unitPx;

      actors_.push_back(std::make_unique<FunctionDeclActor>(*fn, atOrigin(frameOrigin, Rect{0, 0, w, h})));
      byId_[actors_.back()->id()] = actors_.back().get();

      const Vec2 origin = bodyOrigin(frameOrigin, ctx.layout.headerH());
      for (const pt::Node* node : sortedNodes(fn->body)) {
        const Rect bounds = atOrigin(origin, localRect(locationOf(*node), ctx.layout.unitPx));
        actors_.push_back(std::visit(MakeActor{fn->id, bounds}, *node));
        byId_[actors_.back()->id()] = actors_.back().get();
      }
    }
  }

  Actor* GraphScene::topmostAt(Vec2 worldPos) const {
    for (auto it = actors_.rbegin(); it != actors_.rend(); ++it) {
      if ((*it)->bounds().contains(worldPos)) {
        return it->get();
      }
    }
    return nullptr;
  }

  Actor* GraphScene::find(fluir::ID functionId, fluir::ID nodeId) const {
    const auto it = byId_.find(fluir::FullID{functionId, nodeId});
    return it == byId_.end() ? nullptr : it->second;
  }

  Actor* GraphScene::find(fluir::ID functionId) const {
    const auto it = byId_.find(fluir::FullID{functionId});
    return it == byId_.end() ? nullptr : it->second;
  }

}  // namespace fluir::editor
