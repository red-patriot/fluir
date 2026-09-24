#include "editor/transaction/add_conduit.hpp"

#include <algorithm>
#include <memory>
#include <utility>

#include "editor/core/tree_path.hpp"

namespace fluir::editor {
  namespace {

    bool carries(const pt::Conduit& conduit, AddConduit::Endpoint target) {
      return std::ranges::any_of(conduit.children, [&](const pt::Conduit::Output& out) {
        return out.target == target.node && out.index == target.index;
      });
    }

  }  // namespace

  bool AddConduit::execute(pt::ParseTree& tree) {
    replaced_.reset();
    pt::Block* block = blockOf(tree, parent_);
    if (id_ == INVALID_ID || block == nullptr || block->nodes.contains(id_) || block->conduits.contains(id_) ||
        source_.node == target_.node) {
      return false;
    }
    const auto fed =
      std::ranges::find_if(block->conduits, [&](const auto& entry) { return carries(entry.second, target_); });
    if (fed != block->conduits.end()) {
      pt::Conduit& old = fed->second;
      if (old.input == source_.node && old.index == source_.index) {
        return false;
      }
      replaced_ = old;
      std::erase_if(old.children, [&](const pt::Conduit::Output& out) {
        return out.target == target_.node && out.index == target_.index;
      });
      if (old.children.empty()) {
        block->conduits.erase(fed);
      }
    }
    block->conduits.emplace(id_,
                            pt::Conduit{.id = id_,
                                        .input = source_.node,
                                        .index = source_.index,
                                        .children = {{.target = target_.node, .index = target_.index}}});
    return true;
  }

  bool AddConduit::unexecute(pt::ParseTree& tree) {
    pt::Block* block = blockOf(tree, parent_);
    if (block == nullptr || block->conduits.erase(id_) == 0) {
      return false;
    }
    if (replaced_) {
      block->conduits.insert_or_assign(replaced_->id, *replaced_);
    }
    return true;
  }

  std::unique_ptr<Transaction> addConduit(fluir::FullID parent,
                                          fluir::ID newId,
                                          AddConduit::Endpoint source,
                                          AddConduit::Endpoint target) {
    return std::make_unique<AddConduit>(std::move(parent), newId, source, target);
  }

}  // namespace fluir::editor
