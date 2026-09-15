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

    // Constant and Call carry no operand ids.
    template <typename Visit>
    void forOperands(pt::Node& node, Visit visit) {
      std::visit(
        [&visit](auto& n) {
          using T = std::decay_t<decltype(n)>;
          if constexpr (std::is_same_v<T, pt::Binary>) {
            visit(n.lhs);
            visit(n.rhs);
          } else if constexpr (std::is_same_v<T, pt::Unary>) {
            visit(n.lhs);
          }
        },
        node);
    }

  }  // namespace

  bool deleteNode(pt::Block& block, fluir::ID nodeId) {
    if (block.nodes.erase(nodeId) == 0) {
      return false;
    }
    detachConduits(block, nodeId);
    for (auto& [id, node] : block.nodes) {
      forOperands(node, [nodeId](fluir::ID& operand) {
        if (operand == nodeId) {
          operand = fluir::INVALID_ID;
        }
      });
    }
    return true;
  }

  void detachConduits(pt::Block& block, fluir::ID id) {
    std::erase_if(block.conduits, [id](const auto& entry) { return entry.second.input == id; });
    for (const fluir::ID conduitId : stripTargets(block, id)) {
      block.conduits.erase(conduitId);
    }
  }

  bool touches(const pt::Conduit& conduit, fluir::ID nodeId) {
    return conduit.input == nodeId || std::ranges::any_of(conduit.children, [nodeId](const pt::Conduit::Output& out) {
             return out.target == nodeId;
           });
  }

  bool hasOperand(const pt::Node& node, fluir::ID nodeId) {
    bool found = false;
    pt::Node copy = node;
    forOperands(copy, [&found, nodeId](fluir::ID& operand) { found = found || operand == nodeId; });
    return found;
  }

}  // namespace fluir::editor
