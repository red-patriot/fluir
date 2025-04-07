#ifndef FLUIR_COMPILER_AST_AST_HPP
#define FLUIR_COMPILER_AST_AST_HPP

#include "fluir/compiler/ast/declaration.hpp"

namespace fluir::ast {
  struct AST {
    Declarations declarations;
    friend bool operator==(const AST&, const AST&) = default;
  };
}  // namespace fluir::ast

#endif
