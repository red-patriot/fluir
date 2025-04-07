#ifndef FLUIR_COMPILER_AST_DECLARATION_HPP
#define FLUIR_COMPILER_AST_DECLARATION_HPP

#include <memory>
#include <unordered_map>
#include <variant>

#include "fluir/compiler/ast/function_declaration.hpp"

namespace fluir::ast {
  using Declaration = std::variant<FunctionDecl>;

  using Declarations = std::unordered_map<id_t, Declaration>;

}  // namespace fluir::ast

#endif
