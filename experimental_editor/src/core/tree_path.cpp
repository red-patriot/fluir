#include "editor/core/tree_path.hpp"

#include <algorithm>
#include <variant>

namespace fluir::editor {

  pt::Declaration* declarationAt(pt::ParseTree& tree, const FullID& path) {
    if (path.size() != 1) {
      return nullptr;
    }
    const auto it = tree.declarations.find(path.front());
    return it == tree.declarations.end() ? nullptr : &it->second;
  }

  pt::FunctionDecl* functionAt(pt::ParseTree& tree, const FullID& path) {
    return std::get_if<pt::FunctionDecl>(declarationAt(tree, path));
  }

  // Nested containers (loops, conditionals) resolve here once they exist.
  pt::Block* blockOf(pt::ParseTree& tree, const FullID& containerPath) {
    pt::FunctionDecl* fn = functionAt(tree, containerPath);
    return fn == nullptr ? nullptr : &fn->body;
  }

  pt::Node* nodeAt(pt::ParseTree& tree, const FullID& path) {
    if (path.size() < 2) {
      return nullptr;
    }
    pt::Block* block = blockOf(tree, parentOf(path));
    if (block == nullptr) {
      return nullptr;
    }
    const auto it = block->nodes.find(path.back());
    return it == block->nodes.end() ? nullptr : &it->second;
  }

  FlowGraphLocation* locationAt(pt::ParseTree& tree, const FullID& path) {
    if (pt::Declaration* decl = declarationAt(tree, path)) {
      return std::visit([](auto& d) { return &d.location; }, *decl);
    }
    pt::Node* node = nodeAt(tree, path);
    return node == nullptr ? nullptr : std::visit([](auto& n) { return &n.location; }, *node);
  }

  std::string* railTypeAt(pt::FunctionDecl& fn, fluir::ID railId) {
    if (fn.output && fn.output->ret && fn.output->ret->id == railId) {
      return &fn.output->ret->typeName;
    }
    if (!fn.input) {
      return nullptr;
    }
    const auto it = std::ranges::find(fn.input->parameters, railId, &pt::FunctionDecl::Parameter::id);
    return it == fn.input->parameters.end() ? nullptr : &it->typeName;
  }

  FullID parentOf(const FullID& path) { return path.empty() ? FullID{} : FullID(path.begin(), path.end() - 1); }

  // Const overloads share the mutable lookups; none of them writes.
  const pt::Declaration* declarationAt(const pt::ParseTree& tree, const FullID& path) {
    return declarationAt(const_cast<pt::ParseTree&>(tree), path);
  }

  const pt::FunctionDecl* functionAt(const pt::ParseTree& tree, const FullID& path) {
    return functionAt(const_cast<pt::ParseTree&>(tree), path);
  }

  const pt::Block* blockOf(const pt::ParseTree& tree, const FullID& containerPath) {
    return blockOf(const_cast<pt::ParseTree&>(tree), containerPath);
  }

  const pt::Node* nodeAt(const pt::ParseTree& tree, const FullID& path) {
    return nodeAt(const_cast<pt::ParseTree&>(tree), path);
  }

  const FlowGraphLocation* locationAt(const pt::ParseTree& tree, const FullID& path) {
    return locationAt(const_cast<pt::ParseTree&>(tree), path);
  }

  const std::string* railTypeAt(const pt::FunctionDecl& fn, fluir::ID railId) {
    return railTypeAt(const_cast<pt::FunctionDecl&>(fn), railId);
  }

}  // namespace fluir::editor
