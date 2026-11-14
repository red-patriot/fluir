#include "editor/core/scope_path.hpp"

#include <cstddef>
#include <variant>

namespace fluir::editor {

  const pt::Block* blockAt(const pt::ParseTree& tree, const fluir::FullID& scope) {
    // TODO: Make this more generic to handle other control-flow structures
    if (scope.empty() || scope.size() % 2 == 0) {
      return nullptr;
    }
    const auto decl = tree.declarations.find(scope.front());
    const auto* fn = decl == tree.declarations.end() ? nullptr : std::get_if<pt::FunctionDecl>(&decl->second);
    const pt::Block* block = fn == nullptr ? nullptr : &fn->body;
    // Each step down is a conditional in the current block, then one of its branches.
    for (std::size_t i = 1; block != nullptr && i < scope.size(); i += 2) {
      const auto node = block->nodes.find(scope[i]);
      const auto* conditional = node == block->nodes.end() ? nullptr : std::get_if<pt::Conditional>(&node->second);
      if (conditional == nullptr) {
        return nullptr;
      }
      switch (scope[i + 1]) {
        case THEN_BRANCH_ID:
          block = &*conditional->thenScope;
          break;
        case ELSE_BRANCH_ID:
          block = &*conditional->elseScope;
          break;
        default:
          return nullptr;
      }
    }
    return block;
  }

}  // namespace fluir::editor
