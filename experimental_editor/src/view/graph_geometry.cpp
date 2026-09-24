#include "editor/view/graph_geometry.hpp"

#include <algorithm>
#include <unordered_map>
#include <utility>

#include "editor/core/node_access.hpp"

namespace fluir::editor {

  Rect localRect(const FlowGraphLocation& loc, double unitPx) {
    return {static_cast<double>(loc.x) * unitPx,
            static_cast<double>(loc.y) * unitPx,
            static_cast<double>(loc.width) * unitPx,
            static_cast<double>(loc.height) * unitPx};
  }

  Vec2 bodyOrigin(Vec2 functionOrigin, double headerH) { return functionOrigin + Vec2{0.0, headerH}; }

  Rect atOrigin(Vec2 origin, Rect local) { return {origin.x + local.x, origin.y + local.y, local.w, local.h}; }

  Rect dotRect(Vec2 anchor, double terminalDot) {
    return {anchor.x - terminalDot * 0.5, anchor.y - terminalDot * 0.5, terminalDot, terminalDot};
  }

  namespace {

    // Deterministic order: (location.z, id).
    template <typename T>
    std::vector<const T*> sortedByZThenId(const std::unordered_map<fluir::ID, T>& items) {
      std::vector<const T*> out;
      out.reserve(items.size());
      for (const auto& entry : items) {
        out.push_back(&entry.second);
      }
      std::ranges::sort(out, {}, [](const T* item) { return std::pair{locationOf(*item).z, idOf(*item)}; });
      return out;
    }

  }  // namespace

  std::vector<const pt::Declaration*> sortedDeclarations(const pt::ParseTree& tree) {
    return sortedByZThenId(tree.declarations);
  }

  std::vector<const pt::Node*> sortedNodes(const pt::Block& block) { return sortedByZThenId(block.nodes); }

  std::vector<const pt::Conduit*> sortedConduits(const pt::Block& block) {
    std::vector<const pt::Conduit*> conduits;
    conduits.reserve(block.conduits.size());
    for (const auto& entry : block.conduits) {
      conduits.push_back(&entry.second);
    }
    std::sort(
      conduits.begin(), conduits.end(), [](const pt::Conduit* a, const pt::Conduit* b) { return a->id < b->id; });
    return conduits;
  }

}  // namespace fluir::editor
