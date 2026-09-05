#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/actors/actor.hpp"
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

  /** Owns one Actor per function frame and per node, keyed by fluir::FullID (`{functionId}`
   *  for a frame, `{functionId, nodeId}` for a node) since node ids repeat across functions. */
  class GraphScene {
   public:
    void build(const EditorContext& ctx, const pt::ParseTree& tree);

    /** Discards all actors. */
    void clear();

    /** Topmost (last-painted) actor containing worldPos, or nullptr. */
    Actor* topmostAt(Vec2 worldPos) const;

    /** Actor owning node `nodeId` in function `functionId`, or nullptr. */
    Actor* find(fluir::ID functionId, fluir::ID nodeId) const;

    /** Actor owning function `functionId`'s frame, or nullptr. */
    Actor* find(fluir::ID functionId) const;

   private:
    std::vector<std::unique_ptr<Actor>> actors_;
    std::unordered_map<fluir::FullID, NodeActor*, detail::FullIDHash> byId_;
    std::unordered_map<fluir::ID, FunctionDeclActor*> frames_;
  };

}  // namespace fluir::editor
