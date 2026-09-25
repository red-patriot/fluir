#include "editor/core/tree_path.hpp"

#include <algorithm>
#include <variant>

namespace fluir::editor {

  et::Declaration* declarationAt(et::ParseTree& tree, const FullID& path) {
    if (path.size() != 1) {
      return nullptr;
    }
    const auto it = tree.declarations.find(path.front());
    return it == tree.declarations.end() ? nullptr : &it->second;
  }

  et::FunctionDecl* functionAt(et::ParseTree& tree, const FullID& path) {
    return std::get_if<et::FunctionDecl>(declarationAt(tree, path));
  }

  bool isNodePath(const FullID& path) { return path.size() >= 2 && path.size() % 2 == 0; }

  bool isBranchPath(const FullID& path) { return path.size() >= 3 && path.size() % 2 == 1; }

  et::Block* branchBlock(et::Conditional& conditional, fluir::ID branchId) {
    switch (branchId) {
      case THEN_BRANCH_ID:
        return &*conditional.thenScope;
      case ELSE_BRANCH_ID:
        return &*conditional.elseScope;
      default:
        return nullptr;
    }
  }

  et::Block* branchAt(et::ParseTree& tree, const FullID& path) {
    if (!isBranchPath(path)) {
      return nullptr;
    }
    auto* conditional = std::get_if<et::Conditional>(nodeAt(tree, parentOf(path)));
    return conditional == nullptr ? nullptr : branchBlock(*conditional, path.back());
  }

  // Mutually recursive with nodeAt; the recursion terminates on path depth.
  et::Block* blockOf(et::ParseTree& tree, const FullID& containerPath) {
    if (containerPath.size() == 1) {
      et::FunctionDecl* fn = functionAt(tree, containerPath);
      return fn == nullptr ? nullptr : &fn->body;
    }
    return branchAt(tree, containerPath);
  }

  et::Node* nodeAt(et::ParseTree& tree, const FullID& path) {
    if (!isNodePath(path)) {
      return nullptr;
    }
    et::Block* block = blockOf(tree, parentOf(path));
    if (block == nullptr) {
      return nullptr;
    }
    const auto it = block->nodes.find(path.back());
    return it == block->nodes.end() ? nullptr : &it->second;
  }

  FlowGraphLocation* locationAt(et::ParseTree& tree, const FullID& path) {
    if (et::Declaration* decl = declarationAt(tree, path)) {
      return std::visit([](auto& d) { return &d.location; }, *decl);
    }
    if (et::Node* node = nodeAt(tree, path)) {
      return std::visit([](auto& n) { return &n.location; }, *node);
    }
    return nullptr;  // a branch has no geometry of its own: its rect is its conditional's.
  }

  std::string* railTypeAt(et::FunctionDecl& fn, fluir::ID railId) {
    if (fn.output && fn.output->ret && fn.output->ret->id == railId) {
      return &fn.output->ret->typeName;
    }
    if (!fn.input) {
      return nullptr;
    }
    const auto it = std::ranges::find(fn.input->parameters, railId, &et::FunctionDecl::Parameter::id);
    return it == fn.input->parameters.end() ? nullptr : &it->typeName;
  }

  FullID parentOf(const FullID& path) { return path.empty() ? FullID{} : FullID(path.begin(), path.end() - 1); }

  // Const overloads share the mutable lookups; none of them writes.
  const et::Declaration* declarationAt(const et::ParseTree& tree, const FullID& path) {
    return declarationAt(const_cast<et::ParseTree&>(tree), path);
  }

  const et::FunctionDecl* functionAt(const et::ParseTree& tree, const FullID& path) {
    return functionAt(const_cast<et::ParseTree&>(tree), path);
  }

  const et::Block* branchBlock(const et::Conditional& conditional, fluir::ID branchId) {
    return branchBlock(const_cast<et::Conditional&>(conditional), branchId);
  }

  const et::Block* branchAt(const et::ParseTree& tree, const FullID& path) {
    return branchAt(const_cast<et::ParseTree&>(tree), path);
  }

  const et::Block* blockOf(const et::ParseTree& tree, const FullID& containerPath) {
    return blockOf(const_cast<et::ParseTree&>(tree), containerPath);
  }

  const et::Node* nodeAt(const et::ParseTree& tree, const FullID& path) {
    return nodeAt(const_cast<et::ParseTree&>(tree), path);
  }

  const FlowGraphLocation* locationAt(const et::ParseTree& tree, const FullID& path) {
    return locationAt(const_cast<et::ParseTree&>(tree), path);
  }

  const std::string* railTypeAt(const et::FunctionDecl& fn, fluir::ID railId) {
    return railTypeAt(const_cast<et::FunctionDecl&>(fn), railId);
  }

}  // namespace fluir::editor
