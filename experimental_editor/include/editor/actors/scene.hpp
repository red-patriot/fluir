#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/actors/actor.hpp"
#include "editor/components/container_actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  class NodeActor;
  class FunctionDeclActor;

  namespace detail {

    // std::vector has no default std::hash; FullID needs one to key byId_.
    struct FullIDHash {
      std::size_t operator()(const fluir::FullID& id) const noexcept {
        std::size_t seed = id.size();
        for (fluir::ID v : id) {
          seed ^= std::hash<fluir::ID>{}(v) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
        }
        return seed;
      }
    };

  }  // namespace detail

  /** Owns the display tree: an unclipped world root holding one FunctionDeclActor per
   *  function. */
  class GraphScene {
   public:
    void build(const EditorContext& ctx, const pt::ParseTree& tree);

    /** Re-derives every actor's bounds from its live location. */
    void layout(const EditorContext& ctx) const { root_->layout(ctx); }

    /** The world-space root; children are the function frames. */
    Actor& root() const { return *root_; }

    /** Union of the frames' world bounds ({0,0,0,0} when empty). Reflects live
     *  actor bounds, so drags are included without a rebuild. */
    Rect worldBounds() const;

    /** Discards all actors. */
    void clear();

    /** Topmost (last-painted) actor containing worldPos, or nullptr. */
    Actor* topmostAt(Vec2 worldPos) const;

    /** Actor named `nodeId` -- a node or a conduit -- in function `functionId`, or nullptr. */
    Actor* find(fluir::ID functionId, fluir::ID nodeId) const;

    /** Actor owning function `functionId`'s frame, or nullptr. */
    Actor* find(fluir::ID functionId) const;

    /** Marks the actor `id` names as selected; a no-op when it resolves to none. */
    void select(fluir::FullID id);
    void clearSelection();

    /** The selected actor's id: the source of truth a rebuild re-applies. */
    const std::optional<fluir::FullID>& selected() const { return selected_; }

   private:
    /** The actor `id` names -- size 1 is a frame, size 2 a node -- or nullptr. */
    Actor* resolve(const fluir::FullID& id) const;

    // The root spans the whole world: it must not clip or offset its frames.
    std::unique_ptr<ContainerActor> root_ = std::make_unique<ContainerActor>(Rect{0, 0, 0, 0}, Actor::ClipChildren::No);
    std::unordered_map<fluir::FullID, Actor*, detail::FullIDHash> byId_;
    std::unordered_map<fluir::ID, FunctionDeclActor*> frames_;
    std::optional<fluir::FullID> selected_;
  };

}  // namespace fluir::editor
