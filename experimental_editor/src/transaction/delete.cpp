#include "editor/transaction/delete.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>

#include "editor/core/node_access.hpp"
#include "editor/core/tree_edit.hpp"
#include "editor/core/tree_path.hpp"

namespace fluir::editor {
  namespace {

    std::vector<et::Conduit> touchedConduits(const et::Block& block, fluir::ID id) {
      std::vector<et::Conduit> out;
      for (const auto& [conduitId, conduit] : block.conduits) {
        if (touches(conduit, id)) {
          out.push_back(conduit);
        }
      }
      return out;
    }

  }  // namespace

  bool DeleteTransaction::execute(et::ParseTree& tree) {
    if (declarationAt(tree, path_) != nullptr) {
      const auto it = tree.declarations.find(path_.front());
      declaration_ = std::move(it->second);
      tree.declarations.erase(it);
      return true;
    }

    et::Block* block = blockOf(tree, parentOf(path_));
    const et::Node* node = nodeAt(tree, path_);
    if (block == nullptr || node == nullptr) {
      return executeRail(tree);
    }
    // Keep everything the delete will rewrite, as it was, before rewriting it.
    const fluir::ID nodeId = path_.back();
    node_ = *node;
    conduits_ = touchedConduits(*block, nodeId);
    referrers_.clear();
    for (const auto& [id, other] : block->nodes) {
      if (hasOperand(other, nodeId)) {
        referrers_.push_back(other);
      }
    }
    return deleteNode(*block, nodeId);
  }

  bool DeleteTransaction::executeRail(et::ParseTree& tree) {
    et::FunctionDecl* fn = path_.empty() ? nullptr : functionAt(tree, parentOf(path_));
    if (fn == nullptr) {
      return false;
    }
    const fluir::ID id = path_.back();
    if (fn->output && fn->output->ret && fn->output->ret->id == id) {
      ret_ = std::move(fn->output->ret);
      fn->output->ret.reset();
    } else if (fn->input) {
      auto& params = fn->input->parameters;
      const auto it = std::ranges::find(params, id, &et::FunctionDecl::Parameter::id);
      if (it == params.end()) {
        return false;
      }
      paramPos_ = static_cast<std::size_t>(it - params.begin());
      param_ = std::move(*it);
      params.erase(it);
    } else {
      return false;
    }
    conduits_ = touchedConduits(fn->body, id);
    detachConduits(fn->body, id);
    return true;
  }

  bool DeleteTransaction::unexecute(et::ParseTree& tree) {
    if (declaration_) {
      tree.declarations.insert_or_assign(path_.front(), std::move(*declaration_));
      declaration_.reset();
      return true;
    }

    if (param_ || ret_) {
      et::FunctionDecl* fn = functionAt(tree, parentOf(path_));
      if (fn == nullptr || (param_ && !fn->input) || (ret_ && !fn->output)) {
        return false;
      }
      if (param_) {
        auto& params = fn->input->parameters;
        params.insert(params.begin() + static_cast<std::ptrdiff_t>(std::min(paramPos_, params.size())),
                      std::move(*param_));
        param_.reset();
      } else {
        fn->output->ret = std::move(ret_);
        ret_.reset();
      }
      for (et::Conduit& conduit : conduits_) {
        fn->body.conduits.insert_or_assign(conduit.id, std::move(conduit));
      }
      conduits_.clear();
      return true;
    }

    et::Block* block = blockOf(tree, parentOf(path_));
    if (block == nullptr || !node_) {
      return false;
    }
    block->nodes.insert_or_assign(path_.back(), std::move(*node_));
    node_.reset();
    for (et::Conduit& conduit : conduits_) {
      block->conduits.insert_or_assign(conduit.id, std::move(conduit));
    }
    for (et::Node& referrer : referrers_) {
      block->nodes.insert_or_assign(idOf(referrer), std::move(referrer));
    }
    conduits_.clear();
    referrers_.clear();
    return true;
  }

  std::unique_ptr<Transaction> deleteAt(fluir::FullID path) {
    return std::make_unique<DeleteTransaction>(std::move(path));
  }

}  // namespace fluir::editor
