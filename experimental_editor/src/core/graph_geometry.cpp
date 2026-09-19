#include "editor/core/graph_geometry.hpp"

#include <algorithm>
#include <variant>

namespace fluir::editor {

  Rect localRect(const FlowGraphLocation& loc, double unitPx) {
    return {static_cast<double>(loc.x) * unitPx,
            static_cast<double>(loc.y) * unitPx,
            static_cast<double>(loc.width) * unitPx,
            static_cast<double>(loc.height) * unitPx};
  }

  Vec2 bodyOrigin(Vec2 functionOrigin, double headerH) { return functionOrigin + Vec2{0.0, headerH}; }

  Rect atOrigin(Vec2 origin, Rect local) { return {origin.x + local.x, origin.y + local.y, local.w, local.h}; }

  Rect dotRect(Vec2 anchor, double portDot) {
    return {anchor.x - portDot * 0.5, anchor.y - portDot * 0.5, portDot, portDot};
  }

  // Deterministic order: (location.z, id).
  std::vector<const pt::Declaration*> sortedDeclarations(const pt::ParseTree& tree) {
    std::vector<const pt::Declaration*> decls;
    decls.reserve(tree.declarations.size());
    for (const auto& entry : tree.declarations) {
      decls.push_back(&entry.second);
    }
    std::sort(decls.begin(), decls.end(), [](const pt::Declaration* a, const pt::Declaration* b) {
      const auto zOf = [](const pt::Declaration& d) {
        return std::visit([](const auto& decl) { return decl.location.z; }, d);
      };
      const auto idOf = [](const pt::Declaration& d) {
        return std::visit([](const auto& decl) { return decl.id; }, d);
      };
      const int za = zOf(*a);
      const int zb = zOf(*b);
      if (za != zb) {
        return za < zb;
      }
      return idOf(*a) < idOf(*b);
    });
    return decls;
  }

  // Deterministic order: (location.z, id).
  std::vector<const pt::Node*> sortedNodes(const pt::Block& block) {
    std::vector<const pt::Node*> nodes;
    nodes.reserve(block.nodes.size());
    for (const auto& entry : block.nodes) {
      nodes.push_back(&entry.second);
    }
    std::sort(nodes.begin(), nodes.end(), [](const pt::Node* a, const pt::Node* b) {
      const auto zOf = [](const pt::Node& n) {
        return std::visit([](const auto& node) { return node.location.z; }, n);
      };
      const auto idOf = [](const pt::Node& n) { return std::visit([](const auto& node) { return node.id; }, n); };
      const int za = zOf(*a);
      const int zb = zOf(*b);
      if (za != zb) {
        return za < zb;
      }
      return idOf(*a) < idOf(*b);
    });
    return nodes;
  }

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
