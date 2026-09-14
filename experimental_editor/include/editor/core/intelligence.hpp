#pragma once

#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/operator.hpp"

namespace fluir::editor {

  /** Answers what may go where in a tree. Type- and module-aware answers slot in behind the same calls. */
  class Intelligence {
   public:
    /** Operators the operator node at `path` may take; empty for anything else. */
    std::vector<fluir::Operator> operators(const pt::ParseTree& tree, const FullID& path) const;
  };

}  // namespace fluir::editor
