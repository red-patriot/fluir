#include "editor/transaction/delete.hpp"

#include <utility>
#include <variant>

#include "editor/core/tree_edit.hpp"
#include "editor/core/tree_path.hpp"

namespace fluir::editor {
  namespace {

    fluir::ID idOf(const pt::Node& node) {
      return std::visit([](const auto& n) { return n.id; }, node);
    }

  }  // namespace

  bool DeleteTransaction::execute(pt::ParseTree& tree) {
    if (declarationAt(tree, path_) != nullptr) {
      const auto it = tree.declarations.find(path_.front());
      declaration_ = std::move(it->second);
      tree.declarations.erase(it);
      return true;
    }

    pt::Block* block = blockOf(tree, parentOf(path_));
    const pt::Node* node = nodeAt(tree, path_);
    if (block == nullptr || node == nullptr) {
      return false;
    }
    // Keep everything the delete will rewrite, as it was, before rewriting it.
    const fluir::ID nodeId = path_.back();
    node_ = *node;
    conduits_.clear();
    referrers_.clear();
    for (const auto& [id, conduit] : block->conduits) {
      if (touches(conduit, nodeId)) {
        conduits_.push_back(conduit);
      }
    }
    for (const auto& [id, other] : block->nodes) {
      if (hasOperand(other, nodeId)) {
        referrers_.push_back(other);
      }
    }
    return deleteNode(*block, nodeId);
  }

  bool DeleteTransaction::unexecute(pt::ParseTree& tree) {
    if (declaration_) {
      tree.declarations.insert_or_assign(path_.front(), std::move(*declaration_));
      declaration_.reset();
      return true;
    }

    pt::Block* block = blockOf(tree, parentOf(path_));
    if (block == nullptr || !node_) {
      return false;
    }
    block->nodes.insert_or_assign(path_.back(), std::move(*node_));
    node_.reset();
    for (pt::Conduit& conduit : conduits_) {
      block->conduits.insert_or_assign(conduit.id, std::move(conduit));
    }
    for (pt::Node& referrer : referrers_) {
      block->nodes.insert_or_assign(idOf(referrer), std::move(referrer));
    }
    conduits_.clear();
    referrers_.clear();
    return true;
  }

}  // namespace fluir::editor
