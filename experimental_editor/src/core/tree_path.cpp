#include "editor/core/tree_path.hpp"

#include <variant>

namespace fluir::editor {

  pt::FunctionDecl* functionAt(pt::ParseTree& tree, const FullID& path) {
    if (path.size() != 1) {
      return nullptr;
    }
    const auto it = tree.declarations.find(path.front());
    return it == tree.declarations.end() ? nullptr : std::get_if<pt::FunctionDecl>(&it->second);
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
    if (pt::FunctionDecl* fn = functionAt(tree, path)) {
      return &fn->location;
    }
    pt::Node* node = nodeAt(tree, path);
    return node == nullptr ? nullptr : std::visit([](auto& n) { return &n.location; }, *node);
  }

  FullID parentOf(const FullID& path) { return path.empty() ? FullID{} : FullID(path.begin(), path.end() - 1); }

  // Const overloads share the mutable lookups; none of them writes.
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

}  // namespace fluir::editor
