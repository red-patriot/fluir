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

  Vec2 functionOrigin(const FlowGraphLocation& loc, double unitPx) {
    return {static_cast<double>(loc.x) * unitPx, static_cast<double>(loc.y) * unitPx};
  }

  Vec2 bodyOrigin(Vec2 functionOrigin, double headerH) { return functionOrigin + Vec2{0.0, headerH}; }

  Rect atOrigin(Vec2 origin, Rect local) { return {origin.x + local.x, origin.y + local.y, local.w, local.h}; }

  Rect dotRect(Vec2 anchor, double portDot) {
    return {anchor.x - portDot * 0.5, anchor.y - portDot * 0.5, portDot, portDot};
  }

  // Deterministic order: (location.z, id).
  std::vector<const pt::FunctionDecl*> sortedFunctions(const pt::ParseTree& tree) {
    std::vector<const pt::FunctionDecl*> funcs;
    funcs.reserve(tree.declarations.size());
    for (const auto& entry : tree.declarations) {
      if (const auto* fn = std::get_if<pt::FunctionDecl>(&entry.second)) {
        funcs.push_back(fn);
      }
    }
    std::sort(funcs.begin(), funcs.end(), [](const pt::FunctionDecl* a, const pt::FunctionDecl* b) {
      if (a->location.z != b->location.z) {
        return a->location.z < b->location.z;
      }
      return a->id < b->id;
    });
    return funcs;
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

}  // namespace fluir::editor
