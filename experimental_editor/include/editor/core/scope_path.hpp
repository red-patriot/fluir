#ifndef FLUIR_EDITOR_CORE_SCOPE_PATH_HPP
#define FLUIR_EDITOR_CORE_SCOPE_PATH_HPP

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"

namespace fluir::editor {

  // A scope path names a body: `{function}`, then `conditional, branch` pairs down to a branch.
  inline constexpr fluir::ID THEN_BRANCH_ID = 1;
  inline constexpr fluir::ID ELSE_BRANCH_ID = 2;

  /** The block `scope` names in `tree`, or null. */
  const pt::Block* blockAt(const pt::ParseTree& tree, const fluir::FullID& scope);

}  // namespace fluir::editor

#endif
