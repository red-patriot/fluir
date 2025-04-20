#ifndef FLUIR_COMPILER_AST_AST_HPP
#define FLUIR_COMPILER_AST_AST_HPP

#include <vector>

#include "compiler/frontend/ast/declaration.hpp"

namespace fluir::ast {
  template <bool IsGraph>
  struct AbstractSyntax {
    static_assert(IsGraph);  // TODO: Support flattening later...

    std::vector<Declaration> declarations;
  };

  using ASG = AbstractSyntax<true>;
}  // namespace fluir::ast

#endif
