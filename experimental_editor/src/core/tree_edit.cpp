#include "editor/core/tree_edit.hpp"

#include <algorithm>
#include <vector>

namespace fluir::editor {

  namespace {

    // Drops `nodeId` from every conduit's target list, returning the conduits
    // left childless by this edit.
    std::vector<fluir::ID> stripTargets(et::Block& body, fluir::ID nodeId) {
      std::vector<fluir::ID> emptied;
      for (auto& [conduitId, conduit] : body.conduits) {
        const auto removed = std::erase_if(
          conduit.children, [nodeId](const et::Conduit::Output& output) { return output.target == nodeId; });
        if (removed > 0 && conduit.children.empty()) {
          emptied.push_back(conduitId);
        }
      }
      return emptied;
    }

  }  // namespace

  bool deleteNode(et::Block& block, fluir::ID nodeId) {
    if (block.nodes.erase(nodeId) == 0) {
      return false;
    }
    detachConduits(block, nodeId);
    return true;
  }

  void detachConduits(et::Block& block, fluir::ID id) {
    std::erase_if(block.conduits, [id](const auto& entry) { return entry.second.input == id; });
    for (const fluir::ID conduitId : stripTargets(block, id)) {
      block.conduits.erase(conduitId);
    }
  }

  bool touches(const et::Conduit& conduit, fluir::ID nodeId) {
    return conduit.input == nodeId || std::ranges::any_of(conduit.children, [nodeId](const et::Conduit::Output& out) {
             return out.target == nodeId;
           });
  }

}  // namespace fluir::editor
