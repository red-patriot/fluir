#ifndef FLUIR_COMPILER_MODELS_AST_HPP
#define FLUIR_COMPILER_MODELS_AST_HPP

#include <vector>

#include "compiler/models/ast/declaration.hpp"

namespace fluir::ast {
  struct AbstractSyntaxTree {
    std::vector<Declaration> declarations{};
  };

  using AST = AbstractSyntaxTree;
}  // namespace fluir::ast

#endif
