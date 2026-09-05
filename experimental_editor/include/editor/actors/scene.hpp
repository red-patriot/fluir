#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/actors/actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"

namespace fluir::editor {

  /** Owns one Actor per node across `tree`'s functions and supports topmost-first hit
   *  testing. Actors are constructed with absolute (world-space) bounds, in paint order
   *  (matching `sortedFunctions`/`sortedNodes`), via a construction-time factory that picks
   *  the concrete subclass matching each node's `pt::Node` alternative. */
  class GraphScene {
   public:
    /** Discards any previously built actors, then walks `tree`'s functions/nodes (in paint
     *  order) constructing one actor per node with its absolute screen-space bounds. */
    void build(const EditorContext& ctx, const pt::ParseTree& tree);

    /** Discards all actors. */
    void clear();

    /** Returns the topmost (last-painted) actor whose bounds contain `worldPos`, or nullptr
     *  if none do. */
    Actor* topmostAt(Vec2 worldPos) const;

    /** Returns the actor owning node `id`, or nullptr if none does. */
    Actor* find(fluir::ID id) const;

   private:
    std::vector<std::unique_ptr<Actor>> actors_;
    std::unordered_map<fluir::ID, Actor*> byId_;
  };

}  // namespace fluir::editor
