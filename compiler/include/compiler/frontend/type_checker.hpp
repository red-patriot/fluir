#ifndef FLUIR_COMPILER_FRONTEND_TYPE_CHECKER_HPP
#define FLUIR_COMPILER_FRONTEND_TYPE_CHECKER_HPP

#include "compiler/models/ast.hpp"
#include "compiler/utility/context.hpp"

namespace fluir {
  Results<ast::AST> typeCheck(Context& ctx, ast::AST graph);
  Results<ast::Declaration> checkDeclType(Context& ctx, ast::Declaration decl);
}  // namespace fluir

#endif
