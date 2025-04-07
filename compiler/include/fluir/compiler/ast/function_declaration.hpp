#ifndef FLUIR_COMPILER_AST_FUNCTION_DECLARATION_HPP
#define FLUIR_COMPILER_AST_FUNCTION_DECLARATION_HPP

#include <string>

#include "fluir/compiler/ast/block.hpp"
#include "fluir/compiler/ast/utility.hpp"

namespace fluir::ast {
  struct FunctionDecl {
    std::string name;
    fluir::id_t id;
    Block body;
    LocationInfo location;

   private:
    friend bool operator==(const FunctionDecl&, const FunctionDecl&) = default;
  };
}  // namespace fluir::ast

#endif
