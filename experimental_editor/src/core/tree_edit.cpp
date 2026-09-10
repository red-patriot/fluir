#include "editor/core/tree_edit.hpp"

#include <algorithm>
#include <type_traits>
#include <variant>
#include <vector>

namespace fluir::editor {
  namespace {

    // Drops `nodeId` from every conduit's target list, returning the conduits
    // left childless by this edit.
    std::vector<fluir::ID> stripTargets(pt::Block& body, fluir::ID nodeId) {
      std::vector<fluir::ID> emptied;
      for (auto& [conduitId, conduit] : body.conduits) {
        const auto removed = std::erase_if(
          conduit.children, [nodeId](const pt::Conduit::Output& output) { return output.target == nodeId; });
        if (removed > 0 && conduit.children.empty()) {
          emptied.push_back(conduitId);
        }
      }
      return emptied;
    }

    void resetOperands(pt::Node& node, fluir::ID nodeId) {
      std::visit(
        [nodeId](auto& n) {
          using T = std::decay_t<decltype(n)>;
          if constexpr (std::is_same_v<T, pt::Binary>) {
            if (n.lhs == nodeId) {
              n.lhs = fluir::INVALID_ID;
            }
            if (n.rhs == nodeId) {
              n.rhs = fluir::INVALID_ID;
            }
          } else if constexpr (std::is_same_v<T, pt::Unary>) {
            if (n.lhs == nodeId) {
              n.lhs = fluir::INVALID_ID;
            }
          }
          // Constant and Call carry no operand ids.
        },
        node);
    }

  }  // namespace

  bool deleteNode(pt::FunctionDecl& fn, fluir::ID nodeId) {
    if (fn.body.nodes.erase(nodeId) == 0) {
      return false;
    }
    std::erase_if(fn.body.conduits, [nodeId](const auto& entry) { return entry.second.input == nodeId; });
    for (const fluir::ID conduitId : stripTargets(fn.body, nodeId)) {
      fn.body.conduits.erase(conduitId);
    }
    for (auto& [id, node] : fn.body.nodes) {
      resetOperands(node, nodeId);
    }
    return true;
  }

  bool deleteFunction(pt::ParseTree& tree, fluir::ID functionId) {
    // Node ids are body-scoped, so nothing outside the declaration refers in.
    return tree.declarations.erase(functionId) > 0;
  }

}  // namespace fluir::editor
